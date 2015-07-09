#pragma once
#include "ISRVisualisationEngine.h"

namespace LibISR
{
	namespace Engine
	{
		class ISRVisualisationEngine_CPU: public ISRVisualisationEngine
		{
		public:

			void renderObject(Objects::ISRVisualisationState* rendering, const Matrix4f& invH, const Objects::ISRShape_ptr shape, const Vector4f& intrinsic);
			void renderDepth(ISRUShortImage* renderedDepth, Objects::ISRVisualisationState* rendering, const Matrix4f& invH, const Objects::ISRShape_ptr shape, const Vector4f& intrinsic);
			void renderDepthNormalAndObject(ISRUShortImage* renderedDepth, ISRUChar4Image* renderNormal, Objects::ISRVisualisationState* rendering, const Matrix4f& invH, const Objects::ISRShape_ptr shape, const Vector4f& intrinsic);
		};

	}

}