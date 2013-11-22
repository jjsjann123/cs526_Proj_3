/* OpenSceneGraph example, osgvolume.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/
#include "osgvolume.h"

#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/Texture3D>
#include <osg/Texture1D>
#include <osg/ImageSequence>
#include <osg/TexGen>
#include <osg/Geode>
#include <osg/Billboard>
#include <osg/PositionAttitudeTransform>
#include <osg/ClipNode>
#include <osg/ClipPlane>
#include <osg/AlphaFunc>
#include <osg/TexGenNode>
#include <osg/TexEnv>
#include <osg/TexEnvCombine>
#include <osg/Material>
#include <osg/PrimitiveSet>
#include <osg/Endian>
#include <osg/BlendFunc>
#include <osg/BlendEquation>
#include <osg/TransferFunction>
#include <osg/MatrixTransform>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <osgGA/EventVisitor>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/KeySwitchMatrixManipulator>

#include <osgUtil/CullVisitor>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgManipulator/TabBoxDragger>
#include <osgManipulator/TabPlaneTrackballDragger>
#include <osgManipulator/TrackballDragger>

#include <osg/io_utils>

#include <algorithm>
#include <iostream>

#include <osg/ImageUtils>
#include <osgVolume/Volume>
#include <osgVolume/VolumeTile>
#include <osgVolume/RayTracedTechnique>
#include <osgVolume/FixedFunctionTechnique>

#define OMEGA_NO_GL_HEADERS
#include <omega.h>
#include <omegaToolkit.h>
#include <omegaOsg/omegaOsg.h>




osg::Image* createTexture3D(osg::ImageList& imageList,
            unsigned int numComponentsDesired,
            int s_maximumTextureSize,
            int t_maximumTextureSize,
            int r_maximumTextureSize,
            bool resizeToPowerOfTwo)
{

    if (numComponentsDesired==0)
    {
        return osg::createImage3DWithAlpha(imageList,
                                        s_maximumTextureSize,
                                        t_maximumTextureSize,
                                        r_maximumTextureSize,
                                        resizeToPowerOfTwo);
    }
    else
    {
        GLenum desiredPixelFormat = 0;
        switch(numComponentsDesired)
        {
            case(1) : desiredPixelFormat = GL_LUMINANCE; break;
            case(2) : desiredPixelFormat = GL_LUMINANCE_ALPHA; break;
            case(3) : desiredPixelFormat = GL_RGB; break;
            case(4) : desiredPixelFormat = GL_RGBA; break;
        }

        return osg::createImage3D(imageList,
                                        desiredPixelFormat,
                                        s_maximumTextureSize,
                                        t_maximumTextureSize,
                                        r_maximumTextureSize,
                                        resizeToPowerOfTwo);
    }
}

struct ScaleOperator
{
    ScaleOperator():_scale(1.0f) {}
    ScaleOperator(float scale):_scale(scale) {}
    ScaleOperator(const ScaleOperator& so):_scale(so._scale) {}

    ScaleOperator& operator = (const ScaleOperator& so) { _scale = so._scale; return *this; }

    float _scale;

    inline void luminance(float& l) const { l*= _scale; }
    inline void alpha(float& a) const { a*= _scale; }
    inline void luminance_alpha(float& l,float& a) const { l*= _scale; a*= _scale;  }
    inline void rgb(float& r,float& g,float& b) const { r*= _scale; g*=_scale; b*=_scale; }
    inline void rgba(float& r,float& g,float& b,float& a) const { r*= _scale; g*=_scale; b*=_scale; a*=_scale; }
};

struct RecordRowOperator
{
    RecordRowOperator(unsigned int num):_colours(num),_pos(0) {}

    mutable std::vector<osg::Vec4>  _colours;
    mutable unsigned int            _pos;

    inline void luminance(float l) const { rgba(l,l,l,1.0f); }
    inline void alpha(float a) const { rgba(1.0f,1.0f,1.0f,a); }
    inline void luminance_alpha(float l,float a) const { rgba(l,l,l,a);  }
    inline void rgb(float r,float g,float b) const { rgba(r,g,b,1.0f); }
    inline void rgba(float r,float g,float b,float a) const { _colours[_pos++].set(r,g,b,a); }
};

struct WriteRowOperator
{
    WriteRowOperator():_pos(0) {}
    WriteRowOperator(unsigned int num):_colours(num),_pos(0) {}

    std::vector<osg::Vec4>  _colours;
    mutable unsigned int    _pos;

    inline void luminance(float& l) const { l = _colours[_pos++].r(); }
    inline void alpha(float& a) const { a = _colours[_pos++].a(); }
    inline void luminance_alpha(float& l,float& a) const { l = _colours[_pos].r(); a = _colours[_pos++].a(); }
    inline void rgb(float& r,float& g,float& b) const { r = _colours[_pos].r(); g = _colours[_pos].g(); b = _colours[_pos].b(); }
    inline void rgba(float& r,float& g,float& b,float& a) const {  r = _colours[_pos].r(); g = _colours[_pos].g(); b = _colours[_pos].b(); a = _colours[_pos++].a(); }
};

void clampToNearestValidPowerOfTwo(int& sizeX, int& sizeY, int& sizeZ, int s_maximumTextureSize, int t_maximumTextureSize, int r_maximumTextureSize)
{
    // compute nearest powers of two for each axis.
    int s_nearestPowerOfTwo = 1;
    while(s_nearestPowerOfTwo<sizeX && s_nearestPowerOfTwo<s_maximumTextureSize) s_nearestPowerOfTwo*=2;

    int t_nearestPowerOfTwo = 1;
    while(t_nearestPowerOfTwo<sizeY && t_nearestPowerOfTwo<t_maximumTextureSize) t_nearestPowerOfTwo*=2;

    int r_nearestPowerOfTwo = 1;
    while(r_nearestPowerOfTwo<sizeZ && r_nearestPowerOfTwo<r_maximumTextureSize) r_nearestPowerOfTwo*=2;

    sizeX = s_nearestPowerOfTwo;
    sizeY = t_nearestPowerOfTwo;
    sizeZ = r_nearestPowerOfTwo;
}

osg::Image* readRaw(int sizeX, int sizeY, int sizeZ, int numberBytesPerComponent, int numberOfComponents, const std::string& endian, const std::string& raw_filename)
{
    osgDB::ifstream fin(raw_filename.c_str(), std::ifstream::binary);
    if (!fin) return 0;

    GLenum pixelFormat;
    switch(numberOfComponents)
    {
        case 1 : pixelFormat = GL_LUMINANCE; break;
        case 2 : pixelFormat = GL_LUMINANCE_ALPHA; break;
        case 3 : pixelFormat = GL_RGB; break;
        case 4 : pixelFormat = GL_RGBA; break;
        default :
            osg::notify(osg::NOTICE)<<"Error: numberOfComponents="<<numberOfComponents<<" not supported, only 1,2,3 or 4 are supported."<<std::endl;
            return 0;
    }


    GLenum dataType;
    switch(numberBytesPerComponent)
    {
        case 1 : dataType = GL_UNSIGNED_BYTE; break;
        case 2 : dataType = GL_UNSIGNED_SHORT; break;
        case 4 : dataType = GL_UNSIGNED_INT; break;
        default :
            osg::notify(osg::NOTICE)<<"Error: numberBytesPerComponent="<<numberBytesPerComponent<<" not supported, only 1,2 or 4 are supported."<<std::endl;
            return 0;
    }

    int s_maximumTextureSize=256, t_maximumTextureSize=256, r_maximumTextureSize=256;

    int sizeS = sizeX;
    int sizeT = sizeY;
    int sizeR = sizeZ;
    clampToNearestValidPowerOfTwo(sizeS, sizeT, sizeR, s_maximumTextureSize, t_maximumTextureSize, r_maximumTextureSize);

    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->allocateImage(sizeS, sizeT, sizeR, pixelFormat, dataType);


    bool endianSwap = (osg::getCpuByteOrder()==osg::BigEndian) ? (endian!="big") : (endian=="big");

    unsigned int r_offset = (sizeZ<sizeR) ? sizeR/2 - sizeZ/2 : 0;

    int offset = endianSwap ? numberBytesPerComponent : 0;
    int delta = endianSwap ? -1 : 1;
    for(int r=0;r<sizeZ;++r)
    {
        for(int t=0;t<sizeY;++t)
        {
            char* data = (char*) image->data(0,t,r+r_offset);
            for(int s=0;s<sizeX;++s)
            {
                if (!fin) return 0;

                for(int c=0;c<numberOfComponents;++c)
                {
                    char* ptr = data+offset;
                    for(int b=0;b<numberBytesPerComponent;++b)
                    {
                        fin.read((char*)ptr, 1);
                        ptr += delta;
                    }
                    data += numberBytesPerComponent;
                }
            }
        }
    }


    // normalise texture
    {
        // compute range of values
        osg::Vec4 minValue, maxValue;
        osg::computeMinMax(image.get(), minValue, maxValue);
        osg::modifyImage(image.get(),ScaleOperator(1.0f/maxValue.r()));
    }


    fin.close();

    if (dataType!=GL_UNSIGNED_BYTE)
    {
        // need to convert to ubyte

        osg::ref_ptr<osg::Image> new_image = new osg::Image;
        new_image->allocateImage(sizeS, sizeT, sizeR, pixelFormat, GL_UNSIGNED_BYTE);

        RecordRowOperator readOp(sizeS);
        WriteRowOperator writeOp;

        for(int r=0;r<sizeR;++r)
        {
            for(int t=0;t<sizeT;++t)
            {
                // reset the indices to beginning
                readOp._pos = 0;
                writeOp._pos = 0;

                // read the pixels into readOp's _colour array
                osg::readRow(sizeS, pixelFormat, dataType, image->data(0,t,r), readOp);

                // pass readOp's _colour array contents over to writeOp (note this is just a pointer swap).
                writeOp._colours.swap(readOp._colours);

                osg::modifyRow(sizeS, pixelFormat, GL_UNSIGNED_BYTE, new_image->data(0,t,r), writeOp);

                // return readOp's _colour array contents back to its rightful owner.
                writeOp._colours.swap(readOp._colours);
            }
        }

        image = new_image;
    }

    return image.release();


}


osg::TransferFunction1D* readTransferFunctionFile(const std::string& filename, float colorScale=1.0f)
{
    std::string foundFile = osgDB::findDataFile(filename);
    if (foundFile.empty())
    {
        std::cout<<"Error: could not find transfer function file : "<<filename<<std::endl;
        return 0;
    }

    std::cout<<"Reading transfer function "<<filename<<std::endl;

    osg::TransferFunction1D::ColorMap colorMap;
    osgDB::ifstream fin(foundFile.c_str());
    while(fin)
    {
        float value, red, green, blue, alpha;
        fin >> value >> red >> green >> blue >> alpha;
        if (fin)
        {
            std::cout<<"value = "<<value<<" ("<<red<<", "<<green<<", "<<blue<<", "<<alpha<<")"<<std::endl;
            colorMap[value] = osg::Vec4(red*colorScale,green*colorScale,blue*colorScale,alpha*colorScale);
        }
    }

    if (colorMap.empty())
    {
        std::cout<<"Error: No values read from transfer function file: "<<filename<<std::endl;
        return 0;
    }

    osg::TransferFunction1D* tf = new osg::TransferFunction1D;
    tf->assign(colorMap);

    return tf;
}


class TestSupportOperation: public osg::GraphicsOperation
{
public:

    TestSupportOperation():
        osg::GraphicsOperation("TestSupportOperation",false),
        supported(true),
        errorMessage(),
        maximumTextureSize(256) {}

    virtual void operator () (osg::GraphicsContext* gc)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex);

        glGetIntegerv( GL_MAX_3D_TEXTURE_SIZE, &maximumTextureSize );

        osg::notify(osg::NOTICE)<<"Max texture size="<<maximumTextureSize<<std::endl;
    }

    OpenThreads::Mutex  mutex;
    bool                supported;
    std::string         errorMessage;
    GLint               maximumTextureSize;
};

class DraggerVolumeTileCallback : public osgManipulator::DraggerCallback
{
public:

    DraggerVolumeTileCallback(osgVolume::VolumeTile* volume, osgVolume::Locator* locator):
        _volume(volume),
        _locator(locator) {}


    virtual bool receive(const osgManipulator::MotionCommand& command);


    osg::observer_ptr<osgVolume::VolumeTile>    _volume;
    osg::ref_ptr<osgVolume::Locator>            _locator;

    osg::Matrix _startMotionMatrix;

    osg::Matrix _localToWorld;
    osg::Matrix _worldToLocal;

};

bool DraggerVolumeTileCallback::receive(const osgManipulator::MotionCommand& command)
{
    if (!_locator) return false;

    switch (command.getStage())
    {
        case osgManipulator::MotionCommand::START:
        {
            // Save the current matrix
            _startMotionMatrix = _locator->getTransform();

            // Get the LocalToWorld and WorldToLocal matrix for this node.
            osg::NodePath nodePathToRoot;
            osgManipulator::computeNodePathToRoot(*_volume,nodePathToRoot);
            _localToWorld = _startMotionMatrix * osg::computeLocalToWorld(nodePathToRoot);
            _worldToLocal = osg::Matrix::inverse(_localToWorld);

            return true;
        }
        case osgManipulator::MotionCommand::MOVE:
        {
            // Transform the command's motion matrix into local motion matrix.
            osg::Matrix localMotionMatrix = _localToWorld * command.getWorldToLocal()
                                            * command.getMotionMatrix()
                                            * command.getLocalToWorld() * _worldToLocal;

            // Transform by the localMotionMatrix
            _locator->setTransform(localMotionMatrix * _startMotionMatrix);

            // osg::notify(osg::NOTICE)<<"New locator matrix "<<_locator->getTransform()<<std::endl;

            return true;
        }
        case osgManipulator::MotionCommand::FINISH:
        {
            return true;
        }
        case osgManipulator::MotionCommand::NONE:
        default:
            return false;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Python wrapper code.
#ifdef OMEGA_USE_PYTHON
#include "omega/PythonInterpreterWrapper.h"
BOOST_PYTHON_MODULE(myvolume)
{
	// SceneLoader
	PYAPI_REF_BASE_CLASS(myOsgVolume)
		PYAPI_STATIC_REF_GETTER(myOsgVolume, createAndInitialize)
		PYAPI_METHOD(myOsgVolume, setPosition)
		PYAPI_METHOD(myOsgVolume, setRotation)
		PYAPI_METHOD(myOsgVolume, translate)
		PYAPI_METHOD(myOsgVolume, rotate)
		PYAPI_METHOD(myOsgVolume, setScale)
		PYAPI_METHOD(myOsgVolume, activateEffect)

		PYAPI_METHOD(myOsgVolume, setCustomizedProperty)
		PYAPI_METHOD(myOsgVolume, setClipping)
		PYAPI_METHOD(myOsgVolume, setTransferFunction)
		;
		//PYAPI_METHOD(HelloModule, )
		
}
#endif

void myOsgVolume::setPosition( float x, float y, float z)
{
	this->modelForm->setPosition(osg::Vec3f(x, y, z));
}

void myOsgVolume::setRotation( float fx, float fy, float fz, float degree)
{
	this->modelForm->setAttitude(osg::Quat(degree, osg::Vec3f(fx, fy, fz)));
}

void myOsgVolume::translate( float x, float y, float z)
{
	osg::Vec3d pos = this->modelForm->getPosition();
	pos+=osg::Vec3d(x, y, z);
	this->modelForm->setPosition(pos);
}

void myOsgVolume::rotate( float fx, float fy, float fz, float degree)
{
	osg::Quat quat = this->modelForm->getAttitude();
	quat*=osg::Quat(degree,osg::Vec3f(fx, fy, fz));
	this->modelForm->setAttitude(quat);
}

myOsgVolume* myOsgVolume::createAndInitialize()
{
	myOsgVolume* instance = new myOsgVolume();
	ModuleServices::addModule(instance);
	instance->doInitialize(Engine::instance());
	return instance;
}

void myOsgVolume::update(const UpdateContext& context)
{
}

void myOsgVolume::setScale( float x, float y, float z)
{
	_xScale = x;
	_yScale = y;
	_zScale = z;
}

void myOsgVolume::setArguments()
{
	std::cout << "*********************" << std::endl;
	imageFile = "D:\\omegalib\\data\\bmp\\*";
	std::cout << imageFile << std::endl;
	_xScale = 1.0;
	_yScale = 1.0;
	_zScale = 1.0;
	_alpha = 0.02;
	modelForm = new osg::PositionAttitudeTransform();
	modelForm->setPosition(osg::Vec3(-200,-200,-500));
}

void myOsgVolume::setTransferFunction()
{
	std::cout << "t1" << std::endl;
	_tf->clear();
	std::cout << "t2" << std::endl;
	_tf->setColor(0.0, osg::Vec4(0.0,1.0,0.0,0.0));
	std::cout << "t3" << std::endl;
    _tf->setColor(0.5, osg::Vec4(0.0,1.0,1.0,0.5));
    _tf->setColor(1.0, osg::Vec4(0.0,0.0,1.0,1.0));
    std::cout << "t4" << std::endl;
	_volumeTile->setDirty(true);
	std::cout << "t5" << std::endl;
}

void myOsgVolume::setCustomizedProperty()
{
	//osgVolume::AlphaFuncProperty* ap = new osgVolume::AlphaFuncProperty(alphaFunc);
	//osgVolume::SampleDensityProperty* sd = new osgVolume::SampleDensityProperty(0.005);
	//osgVolume::TransparencyProperty* tp = new osgVolume::TransparencyProperty(1.0);
	//osgVolume::TransferFunctionProperty* tfp = transferFunction.valid() ? new osgVolume::TransferFunctionProperty(transferFunction.get()) : 0;

	//{
	//	// Standard
	//	osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
	//	cp->addProperty(ap);
	//	cp->addProperty(sd);
	//	cp->addProperty(tp);
	//	if (tfp) cp->addProperty(tfp);

	//	sp->addProperty(cp);
	//}
}

void myOsgVolume::setClipping()
{
	_volumeTile->setLocator(new osgVolume::Locator(osg::Matrix::translate(0.5, 0, 0)*osg::Matrix::rotate(osg::Quat(0.2, osg::Vec3f(0,1,0)))*osg::Matrix::scale(0.5,0.5,0.5)* (*_matrix)));
}

void myOsgVolume::activateEffect(int index)
{
	//std::cout << index << " " << _volumeTile->getDirty() << std::endl;
	switch(index)
    {
		case(0):	_effectProperty->setActiveProperty(0); break;
        case(1):	_effectProperty->setActiveProperty(1); break;
        case(2):	_effectProperty->setActiveProperty(2); break;
        case(3):	_effectProperty->setActiveProperty(3); break;
		case(4):	break;
		default:	break;
    }
	//std::cout << "And: " << _volumeTile->getDirty() << std::endl;
	//_imageLayer->dirty();
	_volumeTile->setDirty(true);

}


void myOsgVolume::initialize()
{
	this->setArguments();
	int argcT= 1;
	char buffer[10] = "osgvolume";
	char* q = buffer;
	char* argvT[10];
	*argvT = buffer;
	std::cout << "************ test********* " << argvT[0] << std::endl;
	osg::ArgumentParser arguments(&argcT, argvT);


	std::cout << "***********Oh shit********" << std::endl;

    std::string outputFile;
    while (arguments.read("-o",outputFile)) {}

    osg::ref_ptr<osg::TransferFunction1D> transferFunction;
	std::string tranferFunctionFile;
    while (arguments.read("--tf", tranferFunctionFile))
    {
        transferFunction = readTransferFunctionFile(tranferFunctionFile);
    }
    while (arguments.read("--tf-255",tranferFunctionFile))
    {
        transferFunction = readTransferFunctionFile(tranferFunctionFile,1.0f/255.0f);
    }

    //while(arguments.read("--test"))
	if(true)
    {
        transferFunction = new osg::TransferFunction1D;
        transferFunction->setColor(0.0, osg::Vec4(1.0,0.0,0.0,0.0));
        transferFunction->setColor(0.5, osg::Vec4(1.0,1.0,0.0,0.5));
        transferFunction->setColor(1.0, osg::Vec4(0.0,0.0,1.0,1.0));
    }
	_tf = transferFunction;

    while(arguments.read("--test2"))
	{
        transferFunction = new osg::TransferFunction1D;
        transferFunction->setColor(0.0, osg::Vec4(1.0,0.0,0.0,0.0));
        transferFunction->setColor(0.5, osg::Vec4(1.0,1.0,0.0,0.5));
        transferFunction->setColor(1.0, osg::Vec4(0.0,0.0,1.0,1.0));
        transferFunction->assign(transferFunction->getColorMap());
    }

    //{
    //    // deprecated options

    //    bool invalidOption = false;

    //    unsigned int numSlices=500;
    //    while (arguments.read("-s",numSlices)) { OSG_NOTICE<<"Warning: -s option no longer supported."<<std::endl; invalidOption = true; }

    //    float sliceEnd=1.0f;
    //    while (arguments.read("--clip",sliceEnd)) { OSG_NOTICE<<"Warning: --clip option no longer supported."<<std::endl; invalidOption = true; }


    //    if (invalidOption) return;
    //}

	float xMultiplier = this->_xScale;
	float yMultiplier = this->_yScale;
	float zMultiplier = this->_zScale;
	float alphaFunc = this->_alpha;
    //float xMultiplier=1.0;
    //while (arguments.read("--xMultiplier",xMultiplier)) {}

    //float yMultiplier=1.0f;
    //while (arguments.read("--yMultiplier",yMultiplier)) {}

    //float zMultiplier=1.0f;
    //while (arguments.read("--zMultiplier",zMultiplier)) {}


    //float alphaFunc=0.02f;
    //while (arguments.read("--alphaFunc",alphaFunc)) {}

	ShadingModel shadingModel = Standard;
	//ShadingModel shadingModel = Isosurface;
    while(arguments.read("--mip")) shadingModel =  MaximumIntensityProjection;

    while (arguments.read("--isosurface") || arguments.read("--iso-surface")) shadingModel = Isosurface;

    while (arguments.read("--light") || arguments.read("-n")) shadingModel = Light;

    float xSize=0.0f, ySize=0.0f, zSize=0.0f;
    while (arguments.read("--xSize",xSize)) {}
    while (arguments.read("--ySize",ySize)) {}
    while (arguments.read("--zSize",zSize)) {}

    osg::ref_ptr<TestSupportOperation> testSupportOperation = new TestSupportOperation;


    int maximumTextureSize = testSupportOperation->maximumTextureSize;
    int s_maximumTextureSize = maximumTextureSize;
    int t_maximumTextureSize = maximumTextureSize;
    int r_maximumTextureSize = maximumTextureSize;
    while(arguments.read("--maxTextureSize",maximumTextureSize))
    {
        s_maximumTextureSize = maximumTextureSize;
        t_maximumTextureSize = maximumTextureSize;
        r_maximumTextureSize = maximumTextureSize;
    }
    while(arguments.read("--s_maxTextureSize",s_maximumTextureSize)) {}
    while(arguments.read("--t_maxTextureSize",t_maximumTextureSize)) {}
    while(arguments.read("--r_maxTextureSize",r_maximumTextureSize)) {}

    // set up colour space operation.
    osg::ColorSpaceOperation colourSpaceOperation = osg::NO_COLOR_SPACE_OPERATION;
    osg::Vec4 colourModulate(0.25f,0.25f,0.25f,0.25f);
    while(arguments.read("--modulate-alpha-by-luminance")) { colourSpaceOperation = osg::MODULATE_ALPHA_BY_LUMINANCE; }
    while(arguments.read("--modulate-alpha-by-colour", colourModulate.x(),colourModulate.y(),colourModulate.z(),colourModulate.w() )) { colourSpaceOperation = osg::MODULATE_ALPHA_BY_COLOR; }
    while(arguments.read("--replace-alpha-with-luminance")) { colourSpaceOperation = osg::REPLACE_ALPHA_WITH_LUMINANCE; }
    while(arguments.read("--replace-rgb-with-luminance")) { colourSpaceOperation = osg::REPLACE_RGB_WITH_LUMINANCE; }


    enum RescaleOperation
    {
        NO_RESCALE,
        RESCALE_TO_ZERO_TO_ONE_RANGE,
        SHIFT_MIN_TO_ZERO
    };

    RescaleOperation rescaleOperation = RESCALE_TO_ZERO_TO_ONE_RANGE;
    while(arguments.read("--no-rescale")) rescaleOperation = NO_RESCALE;
    while(arguments.read("--rescale")) rescaleOperation = RESCALE_TO_ZERO_TO_ONE_RANGE;
    while(arguments.read("--shift-min-to-zero")) rescaleOperation = SHIFT_MIN_TO_ZERO;


    bool resizeToPowerOfTwo = false;

    unsigned int numComponentsDesired = 0;
    while(arguments.read("--num-components", numComponentsDesired)) {}

    bool useManipulator = false;
    while(arguments.read("--manipulator") || arguments.read("-m")) { useManipulator = true; }


    bool useShader = true;
    while(arguments.read("--shader")) { useShader = true; }
    while(arguments.read("--no-shader")) { useShader = false; }

    bool gpuTransferFunction = true;
    while(arguments.read("--gpu-tf")) { gpuTransferFunction = true; }
    while(arguments.read("--cpu-tf")) { gpuTransferFunction = false; }

    double sampleDensityWhenMoving = 0.0;
    while(arguments.read("--sdwm", sampleDensityWhenMoving)) {}

    while(arguments.read("--lod")) { sampleDensityWhenMoving = 0.02; }

    double sequenceLength = 10.0;
    while(arguments.read("--sequence-duration", sequenceLength) ||
          arguments.read("--sd", sequenceLength)) {}

    typedef std::list< osg::ref_ptr<osg::Image> > Images;
    Images images;


    std::string vh_filename;
    while (arguments.read("--vh", vh_filename))
    {
        std::string raw_filename, transfer_filename;
        int xdim(0), ydim(0), zdim(0);

        osgDB::ifstream header(vh_filename.c_str());
        if (header)
        {
            header >> raw_filename >> transfer_filename >> xdim >> ydim >> zdim >> xSize >> ySize >> zSize;
        }

        if (xdim*ydim*zdim==0)
        {
            std::cout<<"Error in reading volume header "<<vh_filename<<std::endl;
            return;
        }

        if (!raw_filename.empty())
        {
            images.push_back(readRaw(xdim, ydim, zdim, 1, 1, "little", raw_filename));
        }

        if (!transfer_filename.empty())
        {
            osgDB::ifstream fin(transfer_filename.c_str());
            if (fin)
            {
                osg::TransferFunction1D::ColorMap colorMap;
                float value = 0.0;
                while(fin && value<=1.0)
                {
                    float red, green, blue, alpha;
                    fin >> red >> green >> blue >> alpha;
                    if (fin)
                    {
                        colorMap[value] = osg::Vec4(red/255.0f,green/255.0f,blue/255.0f,alpha/255.0f);
                        std::cout<<"value = "<<value<<" ("<<red<<", "<<green<<", "<<blue<<", "<<alpha<<")";
                        std::cout<<"  ("<<colorMap[value]<<")"<<std::endl;
                    }
                    value += 1/255.0;
                }

                if (colorMap.empty())
                {
                    std::cout<<"Error: No values read from transfer function file: "<<transfer_filename<<std::endl;
                    return;
                }

                transferFunction = new osg::TransferFunction1D;
                transferFunction->assign(colorMap);
            }
        }

    }


    int sizeX, sizeY, sizeZ, numberBytesPerComponent, numberOfComponents;
    std::string endian, raw_filename;
    while (arguments.read("--raw", sizeX, sizeY, sizeZ, numberBytesPerComponent, numberOfComponents, endian, raw_filename))
    {
        images.push_back(readRaw(sizeX, sizeY, sizeZ, numberBytesPerComponent, numberOfComponents, endian, raw_filename));
    }
	/*
    int images_pos = arguments.find("--images");
    if (images_pos>=0)
    {
        osg::ImageList imageList;
        int pos=images_pos+1;
        for(;pos<arguments.argc() && !arguments.isOption(pos);++pos)
        {
            std::string arg(arguments[pos]);
            if (arg.find('*') != std::string::npos)
            {
                osgDB::DirectoryContents contents = osgDB::expandWildcardsInFilename(arg);
                for (unsigned int i = 0; i < contents.size(); ++i)
                {
                    osg::Image *image = osgDB::readImageFile( contents[i] );

                    if(image)
                    {
                        OSG_NOTICE<<"Read osg::Image FileName::"<<image->getFileName()<<", pixelFormat=0x"<<std::hex<<image->getPixelFormat()<<std::dec<<", s="<<image->s()<<", t="<<image->t()<<", r="<<image->r()<<std::endl;
                        imageList.push_back(image);
                    }
                }
            }
            else
            {
                // not an option so assume string is a filename.
                osg::Image *image = osgDB::readImageFile( arguments[pos] );

                if(image)
                {
                    OSG_NOTICE<<"Read osg::Image FileName::"<<image->getFileName()<<", pixelFormat=0x"<<std::hex<<image->getPixelFormat()<<std::dec<<", s="<<image->s()<<", t="<<image->t()<<", r="<<image->r()<<std::endl;
                    imageList.push_back(image);
                }
            }
        }

        arguments.remove(images_pos, pos-images_pos);

        // pack the textures into a single texture.
        osg::Image* image = createTexture3D(imageList, numComponentsDesired, s_maximumTextureSize, t_maximumTextureSize, r_maximumTextureSize, resizeToPowerOfTwo);
        if (image)
        {
            images.push_back(image);
        }
        else
        {
            OSG_NOTICE<<"Unable to create 3D image from source files."<<std::endl;
        }
    }
	*/
	osg::ImageList imageList;
	if (imageFile.length() > 0)
    {
		std::string arg = imageFile;
        if (arg.find('*') != std::string::npos)
        {
            osgDB::DirectoryContents contents = osgDB::expandWildcardsInFilename(arg);
            for (unsigned int i = 0; i < contents.size(); ++i)
            {
                osg::Image *image = osgDB::readImageFile( contents[i] );

                if(image)
                {
                    OSG_NOTICE<<"Read osg::Image FileName::"<<image->getFileName()<<", pixelFormat=0x"<<std::hex<<image->getPixelFormat()<<std::dec<<", s="<<image->s()<<", t="<<image->t()<<", r="<<image->r()<<std::endl;
                    imageList.push_back(image);
                }
            }
        }
        else
        {
            // not an option so assume string is a filename.
            osg::Image *image = osgDB::readImageFile( arg );

            if(image)
            {
                OSG_NOTICE<<"Read osg::Image FileName::"<<image->getFileName()<<", pixelFormat=0x"<<std::hex<<image->getPixelFormat()<<std::dec<<", s="<<image->s()<<", t="<<image->t()<<", r="<<image->r()<<std::endl;
                imageList.push_back(image);
            }
        }
    }
	
    // pack the textures into a single texture.
    osg::Image* image = createTexture3D(imageList, numComponentsDesired, s_maximumTextureSize, t_maximumTextureSize, r_maximumTextureSize, resizeToPowerOfTwo);
    if (image)
    {
        images.push_back(image);
    }
    else
    {
        OSG_NOTICE<<"Unable to create 3D image from source files."<<std::endl;
    }


    if (images.empty())
    {
        std::cout<<"No model loaded, please specify a volumetric image file on the command line."<<std::endl;
        return;
    }


    Images::iterator sizeItr = images.begin();
    int image_s = (*sizeItr)->s();
    int image_t = (*sizeItr)->t();
    int image_r = (*sizeItr)->r();
    ++sizeItr;

	std::cout << ">>>>>>>>>>>>>>>>>image size>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
	std::cout << image_s << " " << image_t << " " << image_r << std::endl;

    for(;sizeItr != images.end(); ++sizeItr)
    {
        if ((*sizeItr)->s() != image_s ||
            (*sizeItr)->t() != image_t ||
            (*sizeItr)->r() != image_r)
        {
            std::cout<<"Images in sequence are not of the same dimensions."<<std::endl;
            return;
        }
    }


    osg::ref_ptr<osgVolume::ImageDetails> details = dynamic_cast<osgVolume::ImageDetails*>(images.front()->getUserData());
    osg::ref_ptr<osg::RefMatrix> matrix = details ? details->getMatrix() : dynamic_cast<osg::RefMatrix*>(images.front()->getUserData());

    if (!matrix)
    {
        if (xSize==0.0) xSize = static_cast<float>(image_s);
        if (ySize==0.0) ySize = static_cast<float>(image_t);
        if (zSize==0.0) zSize = static_cast<float>(image_r);

        matrix = new osg::RefMatrix(xSize, 0.0,   0.0,   0.0,
                                    0.0,   ySize, 0.0,   0.0,
                                    0.0,   0.0,   zSize, 0.0,
                                    0.0,   0.0,   0.0,   1.0);
    }
	
    if (xMultiplier!=1.0 || yMultiplier!=1.0 || zMultiplier!=1.0)
    {
        matrix->postMultScale(osg::Vec3d(fabs(xMultiplier), fabs(yMultiplier), fabs(zMultiplier)));
    }

	// AJ set the matrix to class variable
	_matrix = matrix;

    osg::Vec4 minValue(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
    osg::Vec4 maxValue(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
    bool computeMinMax = false;
    for(Images::iterator itr = images.begin();
        itr != images.end();
        ++itr)
    {
        osg::Vec4 localMinValue, localMaxValue;
        if (osg::computeMinMax(itr->get(), localMinValue, localMaxValue))
        {
            if (localMinValue.r()<minValue.r()) minValue.r() = localMinValue.r();
            if (localMinValue.g()<minValue.g()) minValue.g() = localMinValue.g();
            if (localMinValue.b()<minValue.b()) minValue.b() = localMinValue.b();
            if (localMinValue.a()<minValue.a()) minValue.a() = localMinValue.a();

            if (localMaxValue.r()>maxValue.r()) maxValue.r() = localMaxValue.r();
            if (localMaxValue.g()>maxValue.g()) maxValue.g() = localMaxValue.g();
            if (localMaxValue.b()>maxValue.b()) maxValue.b() = localMaxValue.b();
            if (localMaxValue.a()>maxValue.a()) maxValue.a() = localMaxValue.a();

            osg::notify(osg::NOTICE)<<"  ("<<localMinValue<<") ("<<localMaxValue<<") "<<(*itr)->getFileName()<<std::endl;

            computeMinMax = true;
        }
    }

    if (computeMinMax)
    {
        osg::notify(osg::NOTICE)<<"Min value "<<minValue<<std::endl;
        osg::notify(osg::NOTICE)<<"Max value "<<maxValue<<std::endl;

        float minComponent = minValue[0];
        minComponent = osg::minimum(minComponent,minValue[1]);
        minComponent = osg::minimum(minComponent,minValue[2]);
        minComponent = osg::minimum(minComponent,minValue[3]);

        float maxComponent = maxValue[0];
        maxComponent = osg::maximum(maxComponent,maxValue[1]);
        maxComponent = osg::maximum(maxComponent,maxValue[2]);
        maxComponent = osg::maximum(maxComponent,maxValue[3]);

    }


    if (colourSpaceOperation!=osg::NO_COLOR_SPACE_OPERATION)
    {
        for(Images::iterator itr = images.begin();
            itr != images.end();
            ++itr)
        {
            (*itr) = osg::colorSpaceConversion(colourSpaceOperation, itr->get(), colourModulate);
        }
    }

    if (!gpuTransferFunction && transferFunction.valid())
    {
        for(Images::iterator itr = images.begin();
            itr != images.end();
            ++itr)
        {
            *itr = osgVolume::applyTransferFunction(itr->get(), transferFunction.get());
        }
    }

    osg::ref_ptr<osg::Image> image_3d = 0;

    if (images.size()==1)
    {
        osg::notify(osg::NOTICE)<<"Single image "<<images.size()<<" volumes."<<std::endl;
        image_3d = images.front();
    }
    else
    {
        osg::notify(osg::NOTICE)<<"Creating sequence of "<<images.size()<<" volumes."<<std::endl;

        osg::ref_ptr<osg::ImageSequence> imageSequence = new osg::ImageSequence;
        imageSequence->setLength(sequenceLength);
        image_3d = imageSequence.get();
        for(Images::iterator itr = images.begin();
            itr != images.end();
            ++itr)
        {
            imageSequence->addImage(itr->get());
        }
        imageSequence->play();
    }

    osg::ref_ptr<osgVolume::Volume> volume = new osgVolume::Volume;
    osg::ref_ptr<osgVolume::VolumeTile> tile = new osgVolume::VolumeTile;
	_volumeTile = tile;
    volume->addChild(tile.get());

    osg::ref_ptr<osgVolume::ImageLayer> layer = new osgVolume::ImageLayer(image_3d.get());
	_imageLayer = layer;

    if (details)
    {
        layer->setTexelOffset(details->getTexelOffset());
        layer->setTexelScale(details->getTexelScale());
    }

    switch(rescaleOperation)
    {
        case(NO_RESCALE):
            break;

        case(RESCALE_TO_ZERO_TO_ONE_RANGE):
        {
            layer->rescaleToZeroToOneRange();
            break;
        }
        case(SHIFT_MIN_TO_ZERO):
        {
            layer->translateMinToZero();
            break;
        }
    };

    if (xMultiplier<0.0 || yMultiplier<0.0 || zMultiplier<0.0)
    {
        layer->setLocator(new osgVolume::Locator(
            osg::Matrix::translate(xMultiplier<0.0 ? -1.0 : 0.0, yMultiplier<0.0 ? -1.0 : 0.0, zMultiplier<0.0 ? -1.0 : 0.0) *
            osg::Matrix::scale(xMultiplier<0.0 ? -1.0 : 1.0, yMultiplier<0.0 ? -1.0 : 1.0, zMultiplier<0.0 ? -1.0 : 1.0) *
            (*matrix)
            ));;
    }
    else
    {
        //layer->setLocator(new osgVolume::Locator(*matrix));
		layer->setLocator(new osgVolume::Locator(*matrix));
    }
    //tile->setLocator(new osgVolume::Locator(osg::Matrix::scale(0.5,0.5,0.5)* (*matrix)));
	tile->setLocator(new osgVolume::Locator(*matrix));
    
	tile->setLayer(layer.get());

    tile->setEventCallback(new osgVolume::PropertyAdjustmentCallback());

    if (useShader)
    {
		_effectProperty = new osgVolume::SwitchProperty;
		osgVolume::SwitchProperty* sp = _effectProperty;
        sp->setActiveProperty(0);

        osgVolume::AlphaFuncProperty* ap = new osgVolume::AlphaFuncProperty(alphaFunc);
        osgVolume::SampleDensityProperty* sd = new osgVolume::SampleDensityProperty(0.005);
        osgVolume::SampleDensityWhenMovingProperty* sdwm = sampleDensityWhenMoving!=0.0 ? new osgVolume::SampleDensityWhenMovingProperty(sampleDensityWhenMoving) : 0;
        osgVolume::TransparencyProperty* tp = new osgVolume::TransparencyProperty(1.0);
        osgVolume::TransferFunctionProperty* tfp = transferFunction.valid() ? new osgVolume::TransferFunctionProperty(transferFunction.get()) : 0;
		_tfp = tfp;
        {
            // Standard
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
            cp->addProperty(ap);
            cp->addProperty(sd);
            cp->addProperty(tp);
            if (sdwm) cp->addProperty(sdwm);
            if (tfp) cp->addProperty(tfp);

            sp->addProperty(cp);
        }

        {
            // Light
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
            cp->addProperty(ap);
            cp->addProperty(sd);
            cp->addProperty(tp);
            cp->addProperty(new osgVolume::LightingProperty);
            if (sdwm) cp->addProperty(sdwm);
            if (tfp) cp->addProperty(tfp);

            sp->addProperty(cp);
        }

        {
            // Isosurface
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
            cp->addProperty(sd);
            cp->addProperty(tp);
            cp->addProperty(new osgVolume::IsoSurfaceProperty(alphaFunc));
            if (sdwm) cp->addProperty(sdwm);
            if (tfp) cp->addProperty(tfp);

            sp->addProperty(cp);
        }

        {
            // MaximumIntensityProjection
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
            cp->addProperty(ap);
            cp->addProperty(sd);
            cp->addProperty(tp);
            cp->addProperty(new osgVolume::MaximumIntensityProjectionProperty);
            if (sdwm) cp->addProperty(sdwm);
            if (tfp) cp->addProperty(tfp);

            sp->addProperty(cp);
        }

		{
            // CustomizedProperty
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
			cp->addProperty(sd);
            cp->addProperty(tp);
			if (tfp) cp->addProperty(tfp);
			cp->addProperty(ap);
            cp->addProperty(new osgVolume::MaximumIntensityProjectionProperty);
            

            sp->addProperty(cp);
        }

        switch(shadingModel)
        {
            case(Standard):                     sp->setActiveProperty(0); break;
            case(Light):                        sp->setActiveProperty(1); break;
            case(Isosurface):                   sp->setActiveProperty(2); break;
            case(MaximumIntensityProjection):   sp->setActiveProperty(3); break;
        }
		//sp->setActiveProperty(2); work
        layer->addProperty(sp);
		//sp->setActiveProperty(2); work

        tile->setVolumeTechnique(new osgVolume::RayTracedTechnique);
		
    }
    else
    {
        layer->addProperty(new osgVolume::AlphaFuncProperty(alphaFunc));
        tile->setVolumeTechnique(new osgVolume::FixedFunctionTechnique);
    }

    //if (!outputFile.empty())
    //{
    //    std::string ext = osgDB::getFileExtension(outputFile);
    //    std::string name_no_ext = osgDB::getNameLessExtension(outputFile);
    //    if (ext=="osg" || ext=="osgt" || ext=="osgx" )
    //    {
    //        if (image_3d.valid())
    //        {
    //            image_3d->setFileName(name_no_ext + ".dds");
    //            osgDB::writeImageFile(*image_3d, image_3d->getFileName());
    //        }
    //        osgDB::writeNodeFile(*volume, outputFile);
    //    }
    //    else if (ext=="ive" || ext=="osgb" )
    //    {
    //        osgDB::writeNodeFile(*volume, outputFile);
    //    }
    //    else if (ext=="dds")
    //    {
    //        osgDB::writeImageFile(*image_3d, outputFile);
    //    }
    //    else
    //    {
    //        std::cout<<"Extension not support for file output, not file written."<<std::endl;
    //    }

    //    return;
    //}
	
	if (volume.valid())
    {
		osg::ref_ptr<osg::Group> group = new osg::Group;
	    osg::ref_ptr<osg::Node> loadedModel;
		osg::PositionAttitudeTransform* shift = new osg::PositionAttitudeTransform;
		shift->setPosition(osg::Vec3f( -0.5*_xScale*image_s , -0.5*_yScale*image_t, -0.5*_zScale*image_r ));

		myClipNode = new osg::ClipNode;
		osg::ClipPlane * clipPlane = new osg::ClipPlane;

		osg::Plane * plane = new osg::Plane(osg::Vec3d(0,1,0), osg::Vec3d(0,0,0));
		clipPlane->setClipPlane(*plane);

		myClipNode->addClipPlane(clipPlane);
		loadedModel = myClipNode;
		myClipNode->addChild(group);
		group->addChild(shift);
		shift->addChild(volume.get());
		//_effectProperty->setActiveProperty(3);
		activateEffect(2);

/*
        if (useManipulator)
        {
            osg::ref_ptr<osg::Group> group = new osg::Group;

#if 1
            osg::ref_ptr<osgManipulator::Dragger> dragger = new osgManipulator::TabBoxDragger;
#else
            osg::ref_ptr<osgManipulator::Dragger> dragger = new osgManipulator::TrackballDragger();
#endif
            dragger->setupDefaultGeometry();
            dragger->setHandleEvents(true);
            dragger->setActivationModKeyMask(osgGA::GUIEventAdapter::MODKEY_SHIFT);
            dragger->addDraggerCallback(new DraggerVolumeTileCallback(tile.get(), tile->getLocator()));
            dragger->setMatrix(osg::Matrix::translate(0.5,0.5,0.5)*tile->getLocator()->getTransform());

            group->addChild(dragger.get());

            //dragger->addChild(volume.get());

            group->addChild(volume.get());

            loadedModel = group;
        }
*/
        //// set the scene to render
        //viewer.setSceneData(loadedModel.get());

        //// the the viewers main frame loop
        //viewer.run();
		
		modelForm->addChild(loadedModel.get());

		myOsg->setRootNode(modelForm);
		
    }

    return;
}
