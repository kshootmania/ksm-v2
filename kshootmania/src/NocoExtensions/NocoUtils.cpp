#include "NocoUtils.hpp"

namespace NocoUtils
{
	void UnloadExternalTextures()
	{
		noco::Asset::UnloadTexturesIf([](const auto& pair)
		{
			return !pair.first.starts_with(U"imgs/");
		});
	}

	Co::Task<void> WaitForTweenByTag(std::shared_ptr<noco::Canvas> canvas, const char32* tag)
	{
		co_await Co::WaitUntil([canvas, tag]
		{
			return !canvas->isTweenPlayingByTag(tag);
		});
	}

	std::shared_ptr<noco::Node> GetNodeByPath(noco::Canvas* pCanvas, std::initializer_list<const char32_t*> path)
	{
		if (pCanvas == nullptr)
		{
			assert(false && "canvas must not be nullptr");
			return nullptr;
		}

		std::shared_ptr<noco::Node> currentNode = nullptr;
		for (const auto& nodeName : path)
		{
			if (currentNode == nullptr)
			{
				currentNode = pCanvas->findByName(nodeName);
			}
			else
			{
				currentNode = currentNode->findByName(nodeName);
			}

			if (currentNode == nullptr)
			{
				return nullptr;
			}
		}
		return currentNode;
	}

	std::shared_ptr<noco::Node> GetNodeByPath(const std::shared_ptr<noco::Canvas>& pCanvas, std::initializer_list<const char32_t*> path)
	{
		return GetNodeByPath(pCanvas.get(), path);
	}

	std::shared_ptr<noco::Node> GetSubCanvasNodeByName(noco::Canvas* pCanvas, StringView tag, StringView nodeName)
	{
		if (pCanvas == nullptr)
		{
			assert(false && "canvas must not be nullptr");
			return nullptr;
		}

		if (const auto subCanvas = pCanvas->getSubCanvasByTag(tag))
		{
			if (const auto subCanvasCanvas = subCanvas->canvas())
			{
				return subCanvasCanvas->findByName(nodeName);
			}
		}
		return nullptr;
	}
}
