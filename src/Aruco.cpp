#include "Aruco.h"

Aruco::Aruco()
{
}

Aruco::~Aruco()
{
}

void Aruco::createArucoBoard()
{
	int borderBits = 1;

	cv::Size imageSize;
	imageSize.width = markersX * (markerLength + markerSeparation) - markerSeparation + 2 * margins;
	imageSize.height =
		markersY * (markerLength + markerSeparation) - markerSeparation + 2 * margins;

	cv::Ptr<cv::aruco::Dictionary> dictionary =
		getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

	cv::Ptr<cv::aruco::GridBoard> board = cv::aruco::GridBoard::create(
		markersX, markersY, float(markerLength),
		float(markerSeparation), dictionary);

	cv::Mat boardImage;
	board->draw(imageSize, boardImage, margins, borderBits);

	cv::imshow("board", boardImage);
	cv::waitKey(0);

	cv::imwrite("aruco_board.jpg", boardImage);
}

void Aruco::detectBoard()
{
	cv::Mat depthImg;
	Kinect2 kin2;
	CameraParameters camParam;
	TemplateGenerationSettings tempSett;
	readSettings(camParam, tempSett);
	cv::Mat cameraMatrix = camParam.cameraMatrix;
	cv::Mat distCoeffs = camParam.distortionCoefficients;
	cv::Ptr<cv::aruco::Dictionary> dictionary = getPredefinedDictionary(
		cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));
	cv::Ptr<cv::aruco::GridBoard> board = cv::aruco::GridBoard::create(
		markersX, markersY, float(markerLength),
		float(markerSeparation), dictionary);
	uint16_t counter = 0;
	while (true)
	{
		cv::Mat image, imageCopy;
		kin2.getKinectFrames(image, depthImg);
		image.copyTo(imageCopy);
		std::vector<int> ids;
		std::vector<std::vector<cv::Point2f>> corners, rejectedCandidates;
		cv::aruco::detectMarkers(image, dictionary, corners, ids,
		                         cv::aruco::DetectorParameters::create(), rejectedCandidates);
		cv::aruco::refineDetectedMarkers(image, board, corners, ids, rejectedCandidates);
		cv::Matx33d rotMat;
		cv::Vec3d rvec, tvec;
		glm::mat3 glmRotMat;
		cv::Mat cvCast;
		// if at least one marker detected
		if (!ids.empty())
		{
			int valid = cv::aruco::estimatePoseBoard(corners, ids, board, cameraMatrix, distCoeffs,
			                                         rvec, tvec);
			cv::Rodrigues(rvec, rotMat);
			tvec *= 0.283f; //convert to mm
			tvec += rotMat * cv::Vec3d(96, 136, 0);
			//shift cs to center of board. This was measuered by hand and should be adjusted for a new board

			fromCV2GLM(cv::Mat(rotMat), &glmRotMat);
			glm::vec3 eulAng = glm::eulerAngles(glm::toQuat(glmRotMat));
			eulAng.x += CV_PI / 2;
			glmRotMat = glm::toMat4(glm::qua<float>(glm::vec3(eulAng.x, eulAng.y, eulAng.z)));
			cv::putText(imageCopy, glm::to_string(glm::vec3(tvec[0], tvec[1], tvec[2])),
			            cv::Point(50, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5f, cv::Scalar(0, 0, 255),
			            2.0f);
			cv::putText(imageCopy,
			            glm::to_string(glm::degrees(glm::eulerAngles(glm::toQuat(glmRotMat)))),
			            cv::Point(50, 80), cv::FONT_HERSHEY_SIMPLEX, 0.5f, cv::Scalar(0, 255, 255),
			            2.0f);
			fromGLM2CV(glmRotMat, &rotMat);
			cv::Rodrigues(rotMat, rvec);

			if (valid > 0)
				cv::drawFrameAxes(imageCopy, cameraMatrix, distCoeffs, rvec, tvec, 100);
		}

		cv::imshow("Camera", imageCopy);
		auto key = (char)cv::waitKey(1);
		if (key == 9)
		{
			//If TAB is pressed
			std::string picEnding = ".png";
			cv::imwrite("benchmark/img" + std::to_string(counter) + picEnding, image);
			cv::imwrite("benchmark/depth" + std::to_string(counter) + picEnding, depthImg);

			std::string filename = "benchmark/pose" + std::to_string(counter);
			std::string fileEnding = ".yml";
			cv::FileStorage fs(filename + fileEnding, cv::FileStorage::WRITE);
			fs << "rotMat" << rotMat;
			fs << "position" << tvec;
			counter++;
		}
		if (key == 27)
			break;
	}
}
