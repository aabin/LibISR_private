#pragma once

#include "../../Objects/Basic/ISRVisualisationState.h"

#include "../../Objects/Highlevel/ISRFrame.h"
#include "../../Objects/Highlevel/ISRTrackingState.h"
#include "../../Objects/Highlevel/ISRShapeUnion.h"

namespace LibISR
{
	namespace Engine
	{
		class ISRVisualisationEngine
		{
		public:
			ISRVisualisationEngine(){}
			~ISRVisualisationEngine(){}

			//////////////////////////////////////////////////////////////////////////
			//// virtual functions that are implemented both on CPU and on GPU
			//////////////////////////////////////////////////////////////////////////

			virtual void renderObject(Objects::ISRVisualisationState* rendering, const Matrix4f& invH, const Objects::ISRShape_ptr shape, const Vector4f& intrinsic) = 0;
			virtual void renderDepth(ISRUShortImage* renderedDepth, Objects::ISRVisualisationState* rendering, const Matrix4f& invH, const Objects::ISRShape_ptr shape, const Vector4f& intrinsic) = 0;
			virtual void renderDepthNormalAndObject(ISRUShortImage* renderedDepth,ISRUChar4Image* renderNormal, Objects::ISRVisualisationState* rendering, const Matrix4f& invH, const Objects::ISRShape_ptr shape, const Vector4f& intrinsic) = 0;


			void updateMinmaxmImage(ISRFloat2Image* minmaximg, const Matrix4f& H, const Matrix3f& K, const Vector2i& imgsize);
		};
	}
}