#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "FiberBundleLabelSelectCLP.h"

#include "itkPluginFilterWatcher.h"
#include "itkPluginUtilities.h"

// vtkITK includes
#include <vtkITKArchetypeImageSeriesScalarReader.h>

// VTK includes
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTimerLog.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkVersion.h>

// STD includes
#include <iostream>
#include <algorithm>
#include <string>

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  try
  {

  // Label operations
  int includeOperation = 0; // 0-OR; 1-AND
  if (PassOperation == std::string("OR"))
    {
    includeOperation = 0;
    }
  else if (PassOperation == std::string("AND"))
    {
    includeOperation = 1;
    }
  else
    {
    std::cerr << "unknown include operation";
    return EXIT_FAILURE;
    }

  int excludeOperation = 0; // 0-AND; 1-OR
  if (NoPassOperation == std::string("OR"))
    {
    excludeOperation = 0;
    }
  else if (NoPassOperation == std::string("AND"))
    {
    excludeOperation = 1;
    }
  else
    {
    std::cerr << "unknown exclude operation";
    return EXIT_FAILURE;
    }

  // Read in Label volume inputs
  vtkNew<vtkImageCast> imageCastLabel_A;
  vtkNew<vtkITKArchetypeImageSeriesScalarReader> readerLabel_A;
  readerLabel_A->SetArchetype(InputLabel_A.c_str());
  readerLabel_A->SetUseOrientationFromFile(1);
  readerLabel_A->SetUseNativeOriginOn();
  readerLabel_A->SetOutputScalarTypeToNative();
  readerLabel_A->SetDesiredCoordinateOrientationToNative();
  readerLabel_A->SetSingleFile(1);
  readerLabel_A->Update();

  imageCastLabel_A->SetOutputScalarTypeToShort();
#if (VTK_MAJOR_VERSION <= 5)
  imageCastLabel_A->SetInput(readerLabel_A->GetOutput() );
#else
  imageCastLabel_A->SetInputConnection(readerLabel_A->GetOutputPort() );
#endif
  imageCastLabel_A->Update();

  // Read in fiber bundle input to be selected.
  vtkNew<vtkXMLPolyDataReader> readerPD;
  readerPD->SetFileName(InputFibers.c_str());
  readerPD->Update();


  //1. Set up matrices to put fibers into ijk space of volume
  // This assumes fibers are in RAS space of volume (i.e. RAS==world)
  vtkNew<vtkMatrix4x4> Label_A_RASToIJK;
  Label_A_RASToIJK->DeepCopy(readerLabel_A->GetRasToIjkMatrix());

  //the volume we're probing knows its spacing so take this out of the matrix
  double sp[3];
  imageCastLabel_A->GetOutput()->GetSpacing(sp);
  vtkNew<vtkTransform> trans;
  trans->Identity();
  trans->PreMultiply();
  trans->SetMatrix(Label_A_RASToIJK.GetPointer());

  /**
  // Trans from IJK to RAS
  trans->Inverse();
  // Take into account spacing to compute Scaled IJK
  trans->Scale(1/sp[0],1/sp[1],1/sp[2]);
  // now it's RAS to scaled IJK
  trans->Inverse();
 ***/

  // 2. Find polylines
  int inExt[6];
#if (VTK_MAJOR_VERSION <= 5)
  imageCastLabel_A->GetOutput()->GetWholeExtent(inExt);
#else
  imageCastLabel_A->GetOutputInformation(0)->Get(
    vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), inExt);
#endif

  vtkPolyData *input = vtkPolyData::SafeDownCast(readerPD->GetOutput());

  vtkPoints *inPts =input->GetPoints();
  vtkIdType numPts = inPts->GetNumberOfPoints();
  vtkCellArray *inLines = input->GetLines();
  vtkIdType numLines = inLines->GetNumberOfCells();
  vtkIdType npts=0, *pts=NULL;

  if ( !inPts || numPts  < 1 || !inLines || numLines < 1 )
    {
    return EXIT_SUCCESS;
    }

  std::vector<bool> addLines;
  vtkIdType numNewPts = 0;
  vtkIdType numNewCells = 0;
  vtkIdType j;
  double p[3];

  unsigned int label;
  std::vector<bool> passAll;
  for (label=0; label<PassLabel.size(); label++)
    {
    passAll.push_back(false);
    }

  int *labelDims = imageCastLabel_A->GetOutput()->GetDimensions();
  // Check lines
  vtkIdType inCellId;
  for (inCellId=0, inLines->InitTraversal();
       inLines->GetNextCell(npts,pts); inCellId++)
    {
    if (npts < 2)
      {
      addLines.push_back(false);
      std::cerr << "Less than two points in line " << inCellId << std::endl;
      continue; //skip this polyline
      }
    double pIJK[3];
    int pt[3];
    short *inPtr;
    bool addLine = false;
    bool pass = false;
    bool nopass = false;
    for (j=0; j < npts; j++)
      {
      inPts->GetPoint(pts[j],p);
      trans->TransformPoint(p,pIJK);
      pt[0]= (int) floor(pIJK[0]);
      pt[1]= (int) floor(pIJK[1]);
      pt[2]= (int) floor(pIJK[2]);
      if (pt[0] < 0 || pt[1] < 0 || pt[2] < 0 ||
          pt[0] >= labelDims[0] || pt[1] >= labelDims[1] || pt[2] >= labelDims[2])
        {
        std::cerr << "point #" << j <<" on the line #" << inCellId << " is outside the label\n";
        continue;
        }

      inPtr = (short *) imageCastLabel_A->GetOutput()->GetScalarPointer(pt);

      if (excludeOperation == 0) // OR
        {
        for(label=0; label<NotPassLabel.size() && !nopass; label++)
          {
          nopass = (*inPtr == NotPassLabel[label]);
          }
        }
      else if (excludeOperation == 1) // AND
        {
        bool nopassAll = (NotPassLabel.size() > 0);
        for(label=0; label<NotPassLabel.size(); label++)
          {
          nopassAll = nopassAll & (*inPtr == NotPassLabel[label]);
          }
        nopass = nopassAll;
        }
      if (nopass)
        {
        break; //Skip this one, dont check points
        }

      if (PassLabel.size() == 0)
        {
        pass = true;
        }
      else
        {
        if (includeOperation == 0) // OR
          {
          for(label=0; label<PassLabel.size() &&  !pass; label++)
            {
              pass = (*inPtr == PassLabel[label]);
            }
          }
        else if (includeOperation == 1) // AND
          {
          for(label=0; label<PassLabel.size(); label++)
            {
            if (*inPtr == PassLabel[label])
              {
              passAll[label] = true;
              break;
              }
            } // for(label=0;
          } // else if (includeOperation == 1)
        } // if !(PassLabel.size() == 0)
      } //for (j=0; j < npts; j++)

    if (includeOperation == 1 && PassLabel.size() > 0) // AND
      {
      pass = true;
      for (label=0; label<passAll.size(); label++)
        {
        pass = pass & passAll[label];
        }
      }
    addLine = pass && !nopass;

    addLines.push_back(addLine);
    if (addLine)
      {
      // Add line to the output
      numNewPts += npts;
      numNewCells++;
      }
    } //for (inCellId=0, inLines->InitTraversal();

  // Add lines

  //Preallocate PolyData elements
  vtkNew<vtkPolyData> outFibers;

  vtkNew<vtkPoints> points;
  points->Allocate(numNewPts);
  outFibers->SetPoints(points.GetPointer());

  vtkNew<vtkCellArray> outFibersCellArray;
  outFibersCellArray->Allocate(numNewPts+numNewCells);
  outFibers->SetLines(outFibersCellArray.GetPointer());

  //outFibersCellArray->SetNumberOfCells(numNewCells);
  //outFibersCellArray = outFibers->GetLines();

  //vtkIdTypeArray *cellArray=outFibersCellArray->GetData();
  //cellArray->SetNumberOfTuples(numNewPts+numNewCells);

  // if the input has tensors, copy them to the output
  vtkDataArray *oldTensors = input->GetPointData()->GetTensors();
  vtkSmartPointer<vtkFloatArray> newTensors = vtkSmartPointer<vtkFloatArray>::New();
  if (oldTensors)
    {
    newTensors->SetNumberOfComponents(9);
    newTensors->Allocate(9*numNewPts);
    outFibers->GetPointData()->SetTensors(newTensors);
    newTensors = static_cast<vtkFloatArray *> (outFibers->GetPointData()->GetTensors());
    }


  vtkIdType ptId = 0;
  double tensor[9];

  for (inCellId=0, inLines->InitTraversal();
       inLines->GetNextCell(npts,pts); inCellId++)
    {
    if (addLines[inCellId])
      {
      outFibersCellArray->InsertNextCell(npts);
      for (j=0; j < npts; j++)
        {
        inPts->GetPoint(pts[j],p);
        points->InsertPoint(ptId,p);
        outFibersCellArray->InsertCellPoint(ptId);
        if (oldTensors)
          {
          oldTensors->GetTuple(pts[j],tensor);
          newTensors->InsertNextTuple(tensor);
          }
        ptId++;
        }
      }
    }

  //3. Save the output
  vtkNew<vtkXMLPolyDataWriter> writer;
  writer->SetFileName(OutputFibers.c_str());
#if (VTK_MAJOR_VERSION <= 5)
  writer->SetInput(outFibers.GetPointer());
#else
  writer->SetInputData(outFibers.GetPointer());
#endif
  writer->Write();
  }
  catch ( ... )
      {
        std::cerr << "default exception";
        return EXIT_FAILURE;
      }

  return EXIT_SUCCESS;

}
