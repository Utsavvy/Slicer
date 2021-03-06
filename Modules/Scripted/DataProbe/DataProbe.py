import unittest
import qt, vtk, ctk
from __main__ import slicer
import DataProbeLib

#
# DataProbe
#

class DataProbe:
  def __init__(self, parent):
    import string
    parent.title = "DataProbe"
    parent.categories = ["Quantification"]
    parent.contributors = ["Steve Pieper (Isomics)"]
    parent.helpText = string.Template("""
The DataProbe module is used to get information about the current RAS position being indicated by the mouse position.  See <a href=\"$a/Documentation/$b.$c/Modules/DataProbe\">$a/Documentation/$b.$c/Modules/DataProbe</a> for more information.
    """).substitute({ 'a':parent.slicerWikiUrl, 'b':slicer.app.majorVersion, 'c':slicer.app.minorVersion })
    parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, NCIGT, NIH U24 CA180918 (PIs Kikinis and Fedorov) and the Slicer Community.
See <a>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper.
    """
    # TODO: need a DataProbe icon
    #parent.icon = qt.QIcon(':Icons/XLarge/SlicerDownloadMRHead.png')
    self.parent = parent
    self.infoWidget = None

    if slicer.mrmlScene.GetTagByClassName( "vtkMRMLScriptedModuleNode" ) != 'ScriptedModule':
      slicer.mrmlScene.RegisterNodeClass(vtkMRMLScriptedModuleNode())

    # Trigger the menu to be added when application has started up
    if not slicer.app.commandOptions().noMainWindow :
      qt.QTimer.singleShot(0, self.addView);

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['DataProbe'] = self.runTest

  def runTest(self):
    tester = DataProbeTest()
    tester.runTest()

  def __del__(self):
    if self.infoWidget:
      self.infoWidget.removeObservers()

  def addView(self):
    """
    Create the persistent widget shown in the bottom left of the user interface
    Do this in a singleShot callback so the rest of the interface is already
    built.
    """
    # TODO - the parent name will likely change
    try:
      parent = slicer.util.findChildren(text='Data Probe')[0]
    except IndexError:
      print("No Data Probe frame - cannot create DataProbe")
      return
    self.infoWidget = DataProbeInfoWidget(parent,type='small')
    parent.layout().insertWidget(0,self.infoWidget.frame)

class DataProbeInfoWidget(object):

  def __init__(self, parent=None,type='small'):
    self.type = type
    self.nameSize = 24
    # the currentLayoutName is tag on the slice node that corresponds
    # view which should currently be shown in the DataProbe window.
    # Keeping track of this allows us to respond to non-interactor updates
    # to the slice (like from an external tracker) but only in the view where
    # the mouse has most recently entered.
    self.currentLayoutName = None

    self.CrosshairNode = None
    self.CrosshairNodeObserverTag = None

    self.frame = qt.QFrame(parent)
    self.frame.setLayout(qt.QVBoxLayout())

    modulePath = slicer.modules.dataprobe.path.replace("DataProbe.py","")
    self.iconsDIR = modulePath + '/Resources/Icons'

    if type == 'small':
      self.createSmall()

    #Helper class to calculate and display tensor scalars
    self.calculateTensorScalars = CalculateTensorScalars()

    # Observe the crosshair node to get the current cursor position
    self.CrosshairNode = slicer.mrmlScene.GetNthNodeByClass(0, 'vtkMRMLCrosshairNode')
    if self.CrosshairNode:
      self.CrosshairNodeObserverTag = self.CrosshairNode.AddObserver(slicer.vtkMRMLCrosshairNode.CursorPositionModifiedEvent, self.processEvent)


  def __del__(self):
    self.removeObservers()

  def fitName(self,name,nameSize=None):
    if not nameSize:
      nameSize = self.nameSize
    if len(name) > nameSize:
      preSize = nameSize / 2
      postSize = preSize - 3
      name = name[:preSize] + "..." + name[-postSize:]
    return name


  def removeObservers(self):
    # remove observers and reset
    if self.CrosshairNode and self.CrosshairNodeObserverTag:
      self.CrosshairNode.RemoveObserver(self.CrosshairNodeObserverTag)
    self.CrosshairNodeObserverTag = None

  def getPixelString(self,volumeNode,ijk):
    """Given a volume node, create a human readable
    string describing the contents"""
    # TODO: the volume nodes should have a way to generate
    # these strings in a generic way
    if not volumeNode:
      return "No volume"
    imageData = volumeNode.GetImageData()
    if not imageData:
      return "No Image"
    dims = imageData.GetDimensions()
    for ele in xrange(3):
      if ijk[ele] < 0 or ijk[ele] >= dims[ele]:
        return "Out of Frame"
    pixel = ""
    if volumeNode.GetLabelMap():
      labelIndex = int(imageData.GetScalarComponentAsDouble(ijk[0], ijk[1], ijk[2], 0))
      labelValue = "Unknown"
      displayNode = volumeNode.GetDisplayNode()
      if displayNode:
        colorNode = displayNode.GetColorNode()
        if colorNode:
          labelValue = colorNode.GetColorName(labelIndex)
      return "%s (%d)" % (labelValue, labelIndex)

    if volumeNode.IsA("vtkMRMLDiffusionTensorVolumeNode"):
        point_idx = imageData.FindPoint(ijk[0], ijk[1], ijk[2])
        if point_idx == -1:
            return "Out of bounds"

        if not imageData.GetPointData():
            return "No Point Data"

        tensors = imageData.GetPointData().GetTensors()
        if not tensors:
            return "No Tensor Data"

        tensor = imageData.GetPointData().GetTensors().GetTuple9(point_idx)
        scalarVolumeDisplayNode = volumeNode.GetScalarVolumeDisplayNode()

        if scalarVolumeDisplayNode:
            operation = scalarVolumeDisplayNode.GetScalarInvariant()
        else:
            operation = None

        value = self.calculateTensorScalars(tensor, operation=operation)
        if value is not None:
            valueString = ("%f" % value).rstrip('0').rstrip('.')
            return "%s %s"%(scalarVolumeDisplayNode.GetScalarInvariantAsString(), valueString)
        else:
            return scalarVolumeDisplayNode.GetScalarInvariantAsString()

    # default - non label scalar volume
    numberOfComponents = imageData.GetNumberOfScalarComponents()
    if numberOfComponents > 3:
      return "%d components" % numberOfComponents
    for c in xrange(numberOfComponents):
      component = imageData.GetScalarComponentAsDouble(ijk[0],ijk[1],ijk[2],c)
      if component.is_integer():
        component = int(component)
      # format string according to suggestion here:
      # http://stackoverflow.com/questions/2440692/formatting-floats-in-python-without-superfluous-zeros
      # also set the default field width for each coordinate
      componentString = ("%4f" % component).rstrip('0').rstrip('.')
      pixel += ("%s, " % componentString)
    return pixel[:-2]


  def processEvent(self,observee,event):
    # TODO: use a timer to delay calculation and compress events
    insideView = False
    ras = [0.0,0.0,0.0]
    xyz = [0.0,0.0,0.0]
    sliceNode = None
    if self.CrosshairNode:
      insideView = self.CrosshairNode.GetCursorPositionRAS(ras)
      sliceNode = self.CrosshairNode.GetCursorPositionXYZ(xyz)

    sliceLogic = None
    if sliceNode:
      appLogic = slicer.app.applicationLogic()
      if appLogic:
        sliceLogic = appLogic.GetSliceLogic(sliceNode)

    if not insideView or not sliceNode or not sliceLogic:
      self.currentLayoutName = None
      # reset all the readouts
      self.viewerColor.text = ""
      self.viewInfo.text =  ""
      layers = ('L', 'F', 'B')
      for layer in layers:
        self.layerNames[layer].setText( "" )
        self.layerIJKs[layer].setText( "" )
        self.layerValues[layer].setText( "" )
      return

    self.currentLayoutName = sliceNode.GetLayoutName()

    # populate the widgets
    self.viewerColor.setText( " " )
    rgbColor = sliceNode.GetLayoutColor();
    color = qt.QColor.fromRgbF(rgbColor[0], rgbColor[1], rgbColor[2])
    if hasattr(color, 'name'):
      self.viewerColor.setStyleSheet('QLabel {background-color : %s}' % color.name())

    # Described below are the details for the ras coordinate width set to 6:
    #  1: sign
    #  3: suggested number of digits before decimal point
    #  1: decimal point:
    #  1: number of digits after decimal point

    spacing = "%.1f" % sliceLogic.GetLowestVolumeSliceSpacing()[2]
    if sliceNode.GetSliceSpacingMode() == slicer.vtkMRMLSliceNode.PrescribedSliceSpacingMode:
      spacing = "(%s)" % spacing

    self.viewInfo.text = \
      "  {layoutName: <8s}  RAS: ({ras_x:6.1f}, {ras_y:6.1f}, {ras_z:6.1f})  {orient: >8s} Sp: {spacing:s}" \
      .format(layoutName=sliceNode.GetLayoutName(),
              ras_x=ras[0],
              ras_y=ras[1],
              ras_z=ras[2],
              orient=sliceNode.GetOrientationString(),
              spacing=spacing
              )

    def _roundInt(value):
      try:
        return int(round(value))
      except ValueError:
        return 0

    layerLogicCalls = (('L', sliceLogic.GetLabelLayer),
                       ('F', sliceLogic.GetForegroundLayer),
                       ('B', sliceLogic.GetBackgroundLayer))
    for layer,logicCall in layerLogicCalls:
      layerLogic = logicCall()
      volumeNode = layerLogic.GetVolumeNode()
      ijk = [0, 0, 0]
      if volumeNode:
        xyToIJK = layerLogic.GetXYToIJKTransform()
        ijkFloat = xyToIJK.TransformDoublePoint(xyz)
        ijk = [_roundInt(value) for value in ijkFloat]
      self.layerNames[layer].setText(
        "<b>%s</b>" % (self.fitName(volumeNode.GetName()) if volumeNode else "None"))
      self.layerIJKs[layer].setText(
        "({i:4d}, {j:4d}, {k:4d})".format(i=ijk[0], j=ijk[1], k=ijk[2]) if volumeNode else "")
      self.layerValues[layer].setText(
        "<b>%s</b>" % self.getPixelString(volumeNode,ijk) if volumeNode else "")

    sceneName = slicer.mrmlScene.GetURL()
    if sceneName != "":
      self.frame.parent().text = "Data Probe: %s" % self.fitName(sceneName,nameSize=2*self.nameSize)
    else:
      self.frame.parent().text = "Data Probe"

  def createSmall(self):
    """Make the internals of the widget to display in the
    Data Probe frame (lower left of slicer main window by default)"""

    # this method makes SliceView Annotation
    self.sliceAnnotations = DataProbeLib.SliceAnnotations()

    # goto module button
    self.goToModule = qt.QPushButton('->', self.frame)
    self.goToModule.setToolTip('Go to the DataProbe module for more information and options')
    self.frame.layout().addWidget(self.goToModule)
    self.goToModule.connect("clicked()", self.onGoToModule)
    # hide this for now - there's not much to see in the module itself
    self.goToModule.hide()

    # top row - things about the viewer itself
    self.viewerFrame = qt.QFrame(self.frame)
    self.viewerFrame.setLayout(qt.QHBoxLayout())
    self.frame.layout().addWidget(self.viewerFrame)
    self.viewerColor = qt.QLabel(self.viewerFrame)
    self.viewerFrame.layout().addWidget(self.viewerColor)
    self.viewInfo = qt.QLabel()
    self.viewerFrame.layout().addWidget(self.viewInfo)

    self.viewerFrame.layout().addStretch(1)

    def _setFixedFontFamily(widget, family='Monospace'):
      font = widget.font
      font.setFamily(family)
      widget.font = font

    _setFixedFontFamily(self.viewInfo)

    # the grid - things about the layers
    # this method makes labels
    self.layerGrid = qt.QFrame(self.frame)
    layout = qt.QGridLayout()
    self.layerGrid.setLayout(layout)
    self.frame.layout().addWidget(self.layerGrid)
    layers = ('L', 'F', 'B')
    self.layerNames = {}
    self.layerIJKs = {}
    self.layerValues = {}
    for (row, layer) in enumerate(layers):
      col = 0
      layout.addWidget(qt.QLabel(layer), row, col)
      col += 1
      self.layerNames[layer] = qt.QLabel()
      layout.addWidget(self.layerNames[layer], row, col)
      col += 1
      self.layerIJKs[layer] = qt.QLabel()
      layout.addWidget(self.layerIJKs[layer], row, col)
      col += 1
      self.layerValues[layer] = qt.QLabel()
      layout.addWidget(self.layerValues[layer], row, col)
      layout.setColumnStretch(col, 100)

      _setFixedFontFamily(self.layerNames[layer])
      _setFixedFontFamily(self.layerIJKs[layer])
      _setFixedFontFamily(self.layerValues[layer])

    # goto module button
    self.goToModule = qt.QPushButton('->', self.frame)
    self.goToModule.setToolTip('Go to the DataProbe module for more information and options')
    self.frame.layout().addWidget(self.goToModule)
    self.goToModule.connect("clicked()", self.onGoToModule)
    # hide this for now - there's not much to see in the module itself
    self.goToModule.hide()

  def onGoToModule(self):
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('DataProbe')

#
# DataProbe widget
#

class DataProbeWidget:
  """This builds the module contents - nothing here"""
  # TODO: this could have a more in-depth set of information
  # about the volumes and layers in the slice views
  # and possibly other view types as well
  # TODO: Since this is empty for now, it should be hidden
  # from the Modules menu.

  def __init__(self, parent=None):
    self.observerTags = []
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
      self.layout = self.parent.layout()
      self.setup()
      self.parent.show()
    else:
      self.parent = parent
      self.layout = parent.layout()

  def enter(self):
    pass

  def exit(self):
    pass

  def updateGUIFromMRML(self, caller, event):
    pass

  def setup(self):

    # reload button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "DataProbe Reload"
    #self.layout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    #self.layout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    settingsCollapsibleButton = ctk.ctkCollapsibleButton()
    settingsCollapsibleButton.text = "Slice View Annotations Settings"
    self.layout.addWidget(settingsCollapsibleButton)
    settingsVBoxLayout = qt.QVBoxLayout(settingsCollapsibleButton)
    dataProbeInstance = slicer.modules.DataProbeInstance
    if dataProbeInstance.infoWidget:
      sliceAnnotationsFrame = dataProbeInstance.infoWidget.sliceAnnotations.window
      settingsVBoxLayout.addWidget(sliceAnnotationsFrame)

    self.parent.layout().addStretch(1)

  def onReload(self,moduleName="DataProbe"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="DataProbe"):
    self.onReload()
    evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
    tester = eval(evalString)
    tester.runTest()


class CalculateTensorScalars:
    def __init__(self):
        self.dti_math = slicer.vtkDiffusionTensorMathematics()

        self.single_pixel_image = vtk.vtkImageData()
        self.single_pixel_image.SetExtent(0, 0, 0, 0, 0, 0)
        if vtk.VTK_MAJOR_VERSION <= 5:
          self.single_pixel_image.AllocateScalars()

        self.tensor_data = vtk.vtkFloatArray()
        self.tensor_data.SetNumberOfComponents(9)
        self.tensor_data.SetNumberOfTuples(self.single_pixel_image.GetNumberOfPoints())
        self.single_pixel_image.GetPointData().SetTensors(self.tensor_data)

        if vtk.VTK_MAJOR_VERSION <= 5:
          self.dti_math.SetInput(self.single_pixel_image)
        else:
          self.dti_math.SetInputData(self.single_pixel_image)

    def __call__(self, tensor, operation=None):
        if len(tensor) != 9:
            raise ValueError("Invalid tensor a 9-array is required")

        self.tensor_data.SetTupleValue(0, tensor)
        self.tensor_data.Modified()
        self.single_pixel_image.Modified()

        if operation is not None:
            self.dti_math.SetOperation(operation)
        else:
            self.dti_math.SetOperationToFractionalAnisotropy()

        self.dti_math.Update()
        output = self.dti_math.GetOutput()

        if output and output.GetNumberOfScalarComponents() > 0:
            value = output.GetScalarComponentAsDouble(0, 0, 0, 0)
            return value
        else:
            return None


#
# DataProbeLogic
#

class DataProbeLogic:
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass

  def hasImageData(self,volumeNode):
    """This is a dummy logic method that
    returns true if the passed in volume
    node has valid image data
    """
    if not volumeNode:
      print('no volume node')
      return False
    if volumeNode.GetImageData() == None:
      print('no image data')
      return False
    return True


class DataProbeTest(unittest.TestCase):
  """
  This is the test case for your scripted module.
  """

  def delayDisplay(self,message,msec=1000):
    """This utility method displays a small dialog and waits.
    This does two things: 1) it lets the event loop catch up
    to the state of the test so that rendering and widget updates
    have all taken place before the test continues and 2) it
    shows the user/developer/tester the state of the test
    so that we'll know when it breaks.
    """
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    pass

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_DataProbe1()

  def test_DataProbe1(self):
    """ Ideally you should have several levels of tests.  At the lowest level
    tests sould exercise the functionality of the logic with different inputs
    (both valid and invalid).  At higher levels your tests should emulate the
    way the user would interact with your code and confirm that it still works
    the way you intended.
    One of the most important features of the tests is that it should alert other
    developers when their changes will have an impact on the behavior of your
    module.  For example, if a developer removes a feature that you depend on,
    your test should break so they know that the feature is needed.
    """

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    if not slicer.util.getNode('FA'):
      import urllib
      downloads = (
          ('http://slicer.kitware.com/midas3/download?items=5767', 'FA.nrrd', slicer.util.loadVolume),
          )

      for url,name,loader in downloads:
        filePath = slicer.app.temporaryPath + '/' + name
        if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
          print('Requesting download %s from %s...\n' % (name, url))
          urllib.urlretrieve(url, filePath)
        if loader:
          print('Loading %s...\n' % (name,))
          loader(filePath)
    self.delayDisplay('Finished with download and loading\n')

    self.widget = DataProbeInfoWidget()
    self.widget.frame.show()

    self.delayDisplay('Test passed!')
