try: paraview.simple
except: from paraview.simple import *
paraview.simple._DisableFirstRenderCameraReset()

import sys
import math

if len(sys.argv) != 7:
	sys.exit("Usage: python WSS_paraview.py [IN_TXT] [OUT_PNG] [MINWSS] [MAXWSS] [IMGWIDTH] [IMGHEIGHT]")

in_txt = sys.argv[1]
out_png = sys.argv[2]
WSS_min = float(sys.argv[3])
WSS_max = float(sys.argv[4])
IMGWIDTH = int(sys.argv[5])
IMGHEIGHT = int(sys.argv[6])

infile = CSVReader( FileName=[in_txt] )
infile.FieldDelimiterCharacters = ' '
infile.HaveHeaders = 0

SpreadSheetView1 = GetRenderView()

TableToPoints1 = TableToPoints()
TableToPoints1.XColumn = 'Field 0'
TableToPoints1.YColumn = 'Field 1'
TableToPoints1.ZColumn = 'Field 2'

RenderView2 = CreateRenderView()
RenderView2.CompressorConfig = 'vtkSquirtCompressor 0 3'
RenderView2.UseLight = 1
RenderView2.LightSwitch = 0
RenderView2.OrientationAxesVisibility = 0
RenderView2.CameraViewUp = [-0.013032430490094556, 0.08979497030386574, 0.995875001726446]
RenderView2.Background = [1.0, 1.0, 1.0]
RenderView2.CameraFocalPoint = [0.03445, 0.027325, 0.019437499999999996]
RenderView2.CenterAxesVisibility = 0
RenderView2.CameraParallelScale = 0.048033486040990205
RenderView2.CenterOfRotation = [0.03445, 0.027325, 0.0194375]
RenderView2.CameraPosition = [0.026369226950367693, -0.05478092735861384, 0.14873586630350752]
RenderView2.CameraClippingRange = [0.08658648785019117, 0.2378242589228463]

RenderView2.ViewSize = [IMGWIDTH, IMGHEIGHT]


Glyph1 = Glyph( GlyphType="2D Glyph", GlyphTransform="Transform2" )
Glyph1.Scalars = ['POINTS', '']
Glyph1.SetScaleFactor = 0.006885
Glyph1.GlyphType.GlyphType = 'Square'
Glyph1.GlyphTransform.Scale = [0.043, 0.043, 0.043]
Glyph1.MaximumNumberofPoints = 200000

DataRepresentation7 = GetDisplayProperties(Glyph1)
DataRepresentation7.DiffuseColor = [0.0, 0.0, 0.0]

DataRepresentation1 = Show()
DataRepresentation1.EdgeColor = [0.0, 0.0, 0.0]
#DataRepresentation1.SelectionPointFieldDataArrayName = 'Field 3'
#DataRepresentation1.ColorArrayName = ('POINT_DATA', 'Field 3')
DataRepresentation1.ScaleFactor = 0.006885000000000001


# Do it again...
infile2 = CSVReader( FileName=[in_txt] )
infile2.FieldDelimiterCharacters = ' '
infile2.HaveHeaders = 0

SpreadSheetView2 = GetRenderView()

TableToPoints2 = TableToPoints()
TableToPoints2.XColumn = 'Field 0'
TableToPoints2.YColumn = 'Field 1'
TableToPoints2.ZColumn = 'Field 2'

DataRepresentation2 = Show()
DataRepresentation2.EdgeColor = [0.0, 0.0, 0.5000076295109483]
DataRepresentation2.SelectionPointFieldDataArrayName = 'Field 3'
DataRepresentation2.ColorArrayName = ('POINT_DATA', 'Field 3')
DataRepresentation2.ScaleFactor = 0.006885000000000001

AnimationScene1 = GetAnimationScene()
AnimationScene1.ViewModules = [ SpreadSheetView1, RenderView2 ]

WSS_min = math.floor(WSS_min)
WSS_max = math.ceil(WSS_max)

WSS_range = WSS_max - WSS_min

if WSS_min > 0:
	a1_Field3_PVLookupTable = GetLookupTableForArray( "Field 3", 1, NanColor=[0.0, 0.498039, 1.0],
	RGBPoints=[	WSS_min + WSS_range * 0.00, 0.0,      0.0,      0.0,
			WSS_min + WSS_range * 0.33, 0.901961, 0.0,      0.0,
			WSS_min + WSS_range * 0.66, 0.901961, 0.901961, 0.0,
			WSS_min + WSS_range * 1.00, 1.0,      1.0,      1.0
	], ColorSpace='RGB')
else:
	a1_Field3_PVLookupTable = GetLookupTableForArray( "Field 3", 1, NanColor=[1.0, 1.0, 0.0],
	RGBPoints=[	WSS_min + WSS_range * 0.00, 0.0, 1.0, 1.0,
			WSS_min + WSS_range * 0.45, 0.0, 0.0, 1.0,
			WSS_min + WSS_range * 0.50, 0.0, 0.0, 0.501961,
			WSS_min + WSS_range * 0.55, 1.0, 0.0, 0.0,
			WSS_min + WSS_range * 1.00, 1.0, 1.0, 0.0
	], ColorSpace='RGB' )


ScalarBarWidgetRepresentation1 = CreateScalarBar( Title='WSS', LabelFontSize=25, Enabled=1, LookupTable=a1_Field3_PVLookupTable, TitleFontSize=25 )
GetRenderView().Representations.append(ScalarBarWidgetRepresentation1)

ScalarBarWidgetRepresentation1.LabelColor=[0.,0.,0.]

DataRepresentation2.LookupTable = a1_Field3_PVLookupTable

WriteImage(out_png)

Render()
