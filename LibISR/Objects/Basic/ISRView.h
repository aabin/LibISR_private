#pragma once
#include "../../Utils/LibISRDefine.h"

#include "ISRHistogram.h"
#include "ISRCalib.h"

namespace LibISR
{
	/**
	\brief
		view carries the basic image information
		including RGB, depth, raw depth and aligned RGB
		also includes the calibration parameters

		refactored: Jan/13/2015
	*/

	namespace Objects
	{

		class ISRView
		{
		public:
			enum InputDepthType
			{
				ISR_DISPARITY_DEPTH,
				ISR_SHORT_DEPTH,
			};

			InputDepthType inputDepthType;

			ISRCalib *calib;

			ISRUChar4Image *rgb;
			ISRShortImage *rawDepth;

			ISRFloatImage *depth;
			ISRUChar4Image *alignedRgb;

			ISRView(const ISRCalib &calib, Vector2i rgb_size, Vector2i d_size, bool  useGPU = false)
			{
				this->calib = new ISRCalib(calib);
				this->rgb = new ISRUChar4Image(rgb_size, useGPU);
				this->rawDepth = new ISRShortImage(d_size, useGPU);
				this->depth = new ISRFloatImage(d_size, useGPU);
				this->alignedRgb = new ISRUChar4Image(d_size, useGPU);
			}

			~ISRView()
			{
				delete calib;

				delete rgb;
				delete depth;

				delete rawDepth;
				delete alignedRgb;
			}

			ISRView(const ISRView&);
			ISRView& operator=(const ISRView&);
		};
	}
}

