#pragma once
#include <vector>

#include "defines.h"
#include "model_buffer.h"
#include "utility.h"
#include "High_Level_Linemod.h"
#include "Opengl_Render.h"
#include "Camera_View_Points.h"

class Template_Generator
{
public:
	Template_Generator(CameraParameters const& in_camParameters, TemplateGenerationSettings const& in_templateSettings);
	~Template_Generator();
	void run();

private:
	OpenGLRender* opengl;
	HighLevelLinemod* line;
	std::vector<glm::vec3> camVertices;

	std::vector<std::string> modelFiles;
	std::string modelFolder;
	float32 startDistance;
	float32 endDistance;
	float32 stepSize;
	uint16 subdivisions;

	uint32 numCameraVertices;

	void createCamViewPoints(float32 in_radiusToModel);
	void renderImages(std::vector<cv::Mat>& in_imgVec, uint16 in_modelIterator, uint16 in_vertIterator);

	void printProgBar(uint16 in_percent, std::string in_mfile);
	uint16 calculateCurrentPercent(uint16 const& in_spehreRadius, uint16 const& in_currentIteration);
};

