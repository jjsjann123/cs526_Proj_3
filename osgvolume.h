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
    MaximumIntensityProjection
};

using namespace omega;
using namespace omegaToolkit;
using namespace omegaOsg;

class myOsgVolume : public EngineModule
{
public:
	myOsgVolume(std::string filename, float alpha, float fx, float fy, float fz) 
		: EngineModule("OsgViewer"),
		_xScale(fx),
		_yScale(fy),
		_zScale(fz),
		_alpha(alpha),
		imageFile(filename)
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
	void addTransferPoint(float intensity, float r, float g, float b, float alpha);
	void clearTransferFunction();
	void setClipping();

	void setAlphaFunc(float alpha);
	void setScale(float x, float y, float z);
	void setSampleDensity(float sd);
	void setTransparency(float tp);

	void setDirty();
	
	//setup
	static myOsgVolume* createAndInitialize(std::string filename, float alpha = 0.02, float fx=1, float fy=1, float fz=1);
	//virtual void update(const UpdateContext&context);

private:
	Ref<OsgModule> myOsg;
	Ref<osg::ClipNode> myClipNode;

	Ref<osgVolume::VolumeTile> _volumeTile;
	Ref<osgVolume::ImageLayer> _imageLayer;
	
	// property 4 -> osgVolume::AlphaFuncProperty* ap = new osgVolume::AlphaFuncProperty(alphaFunc);
	// property 5 -> cp->addProperty(new osgVolume::MaximumIntensityProjectionProperty);
	// property 3 -> osgVolume::TransferFunctionProperty* tfp = transferFunction.valid() ? new osgVolume::TransferFunctionProperty(transferFunction.get()) : 0;
	Ref<osgVolume::SwitchProperty> _effectProperty;
	Ref<osgVolume::CompositeProperty> _customProperty;
	Ref<osgVolume::AlphaFuncProperty> _ap;
	Ref<osgVolume::IsoSurfaceProperty> _is;
	Ref<osgVolume::SampleDensityProperty> _sd;
	Ref<osgVolume::TransparencyProperty> _tp;
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
	float _sampleDensity;
	float _transparency;
};

#endif