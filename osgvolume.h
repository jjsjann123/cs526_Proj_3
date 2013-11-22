#ifndef	__AJ_OSGVOLUME__
#define __AJ_OSGVOLUME__

#include "cyclops/SceneManager.h"

#include <osg/ClipNode>
#include <osgVolume/Volume>

enum ShadingModel
{
    Standard,
    Light,
    Isosurface,
    MaximumIntensityProjection,
	Customized
};

using namespace omega;
using namespace omegaToolkit;
using namespace omegaOsg;

class myOsgVolume : public EngineModule
{
public:
	myOsgVolume() : EngineModule("OsgViewer")
	{
		//myOsg = new OsgModule();
		//ModuleServices::addModule(myOsg);
		myOsg = OsgModule::instance();
	}

	
	virtual void initialize();
	virtual void update(const UpdateContext& context);
	void setArguments();

	// Interfaces
	void setPosition( float x, float y, float z );
	void setRotation( float fx, float fy, float fz, float degree);
	void translate( float x, float y, float z);
	void rotate( float fx, float fy, float fz, float degree);
	//void activateEffect( ShadingModel index );
	void activateEffect( int index );
	
	void setCustomizedProperty();
	void setTransferFunction();
	void setClipping();
	
	//setup
	static myOsgVolume* createAndInitialize();
	void setScale(float x, float y, float z);
	//virtual void update(const UpdateContext&context);

private:
	Ref<OsgModule> myOsg;
	Ref<osg::ClipNode> myClipNode;
	Ref<osgVolume::SwitchProperty> _effectProperty;
	Ref<osgVolume::VolumeTile> _volumeTile;
	Ref<osgVolume::ImageLayer> _imageLayer;
	
	// property 4 -> osgVolume::AlphaFuncProperty* ap = new osgVolume::AlphaFuncProperty(alphaFunc);
	// property 5 -> cp->addProperty(new osgVolume::MaximumIntensityProjectionProperty);
	// property 3 -> osgVolume::TransferFunctionProperty* tfp = transferFunction.valid() ? new osgVolume::TransferFunctionProperty(transferFunction.get()) : 0;
	Ref<osgVolume::CompositeProperty> _customProperty;
	Ref<osgVolume::TransferFunctionProperty> _tfp;
	Ref<osg::TransferFunction1D> _tf;


	Ref<osg::RefMatrix> _matrix;
	
	//Ref<SceneManager> mySceneManager;
	osg::PositionAttitudeTransform* modelForm;
	std::string imageFile;
	float _xScale;
	float _yScale;
	float _zScale;
	float _alpha;
};

#endif