﻿#pragma once

namespace Sentry
{
				class IFrameUpdate
				{
				public:
								virtual void OnFrameUpdate(float t) = 0;
				};

				class ISystemUpdate
				{
				public:
								virtual void OnSystemUpdate() = 0;
				};

				class ISecondUpdate
				{
				public:
								virtual void OnSecondUpdate() = 0;
				};

				class ILastFrameUpdate
				{
				public:
								virtual void OnLastFrameUpdate() = 0;
				};

				class INetSystemUpdate
				{
				public:
								virtual void OnNetSystemUpdate(AsioContext & io) = 0;
				};
}