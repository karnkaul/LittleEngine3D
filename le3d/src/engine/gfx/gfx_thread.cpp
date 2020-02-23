#include <condition_variable>
#include <deque>
#include <mutex>
#include <glad/glad.h>
#include "le3d/defines.hpp"
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/env/threads.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/gfx_thread.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "engine/context_impl.hpp"

namespace le
{
namespace
{
class DoubleBufferRenderer final
{
public:
	using Buffer = std::deque<gfx::Deferred>;

public:
	std::atomic_bool m_bWork = false;
	std::atomic_bool m_bReady = false;

public:
	std::mutex m_renderMutex;
	std::condition_variable m_renderDone;
	bool m_bBusy = false;
	HThread m_hWorker = 0;

	Buffer m_bufA;
	Buffer m_bufB;
	Buffer* m_pRenderBuf = &m_bufA;
	Buffer* m_pEnqueueBuf = &m_bufB;

public:
	void start();
	void stop();
	void present();

	bool isIdle() const;
	bool isRunning() const;

private:
	void work();
	void swap();
	void wait();
	void render(Buffer& buffer);
};

#if defined(LE3D_HEAVY_RENDER_TOGGLE)
bool g_bForceHeavyRender = false;
#endif
DoubleBufferRenderer g_renderer;
GFXMode g_mode = GFXMode::ImmediateMainThread;

void DoubleBufferRenderer::start()
{
	context::releaseContextThread();
	m_bReady = false;
	m_bWork = true;
	m_hWorker = threads::newThread([this]() { work(); });
	LOG_I("[%s] Starting Render Thread ...", typeName(*this).data());
	while (!m_bReady)
	{
		std::this_thread::yield();
	}
	return;
}

void DoubleBufferRenderer::stop()
{
	LOG_I("[%s] Stopping Render Thread ...", typeName(*this).data());
	m_bWork = false;
	threads::join(m_hWorker);
	context::setContextThread();
	if (!m_pRenderBuf->empty())
	{
		auto buffer = std::move(*m_pRenderBuf);
		render(buffer);
	}
	return;
}

void DoubleBufferRenderer::present()
{
	if (m_hWorker > 0 && m_bWork)
	{
		wait();
	}
	else
	{
		auto buffer = std::move(*m_pRenderBuf);
		render(buffer);
	}
	swap();
	return;
}

bool DoubleBufferRenderer::isIdle() const
{
	return m_hWorker == 0;
}

bool DoubleBufferRenderer::isRunning() const
{
	return m_hWorker > 0 && m_bWork;
}

void DoubleBufferRenderer::work()
{
	context::setContextThread();
	LOG_I("[%s] ... Render Thread Started", typeName(*this).data());
	m_bReady = true;
	while (m_bWork)
	{
		std::unique_lock<std::mutex> lock(m_renderMutex);
		m_bBusy = false;
		while (m_bWork && m_pRenderBuf->empty())
		{
			lock.unlock();
			m_renderDone.notify_one();
			std::this_thread::yield();
			lock.lock();
		}
		if (!m_bWork)
		{
			break;
		}
		m_bBusy = true;
		auto buffer = std::move(*m_pRenderBuf);
		lock.unlock();
		render(buffer);
	}
	context::releaseContextThread();
	LOG_I("[%s] ... Render Thread Stopped", typeName(*this).data());
	return;
}

void DoubleBufferRenderer::swap()
{
	std::lock_guard<std::mutex> lock(m_renderMutex);
	ASSERT(m_pRenderBuf->empty(), "Non-empty Render Buffer!");
	std::swap(m_pRenderBuf, m_pEnqueueBuf);
	return;
}

void DoubleBufferRenderer::wait()
{
	std::unique_lock<std::mutex> lock(m_renderMutex);
	m_renderDone.wait(lock, [this]() -> bool { return !m_bBusy && m_pRenderBuf->empty(); });
	ASSERT(!m_bBusy, "Invariant violated!");
	return;
}

void DoubleBufferRenderer::render(Buffer& buffer)
{
	for (auto& task : buffer)
	{
		cxChk();
		task();
	}
	return;
}
} // namespace

bool gfx::setMode(GFXMode mode)
{
	if (g_mode != mode)
	{
		switch (mode)
		{
		default:
			break;
		case GFXMode::BufferedThreaded:
		{
			if (g_renderer.isIdle())
			{
				g_renderer.start();
			}
			LOG_D("[%s] GFXMode set to BufferedThreaded", typeName<DoubleBufferRenderer>().data());
			break;
		}
		case GFXMode::BufferedMainThread:
		{
			if (g_renderer.isRunning())
			{
				g_renderer.stop();
			}
			LOG_D("[%s] GFXMode set to BufferedMainThread", typeName<DoubleBufferRenderer>().data());
			break;
		}
		case GFXMode::ImmediateMainThread:
		{
			if (g_renderer.isRunning())
			{
				g_renderer.stop();
			}
			std::lock_guard<std::mutex> lock(g_renderer.m_renderMutex);
			auto pEnqueueBuf = g_renderer.m_pEnqueueBuf;
			for (auto& task : *pEnqueueBuf)
			{
				cxChk();
				task();
			}
			LOG_D("[%s] GFXMode set to ImmediateMainThread", typeName<DoubleBufferRenderer>().data());
			break;
		}
		}
		g_mode = mode;
		return true;
	}
	return false;
}

GFXMode gfx::mode()
{
	return g_mode;
}

void gfx::enqueue(Deferred task)
{
	switch (g_mode)
	{
	default:
	case GFXMode::BufferedMainThread:
	case GFXMode::BufferedThreaded:
	{
		std::lock_guard<std::mutex> lock(g_renderer.m_renderMutex);
		g_renderer.m_pEnqueueBuf->push_back(std::move(task));
		break;
	}
	case GFXMode::ImmediateMainThread:
	{
		cxChk();
		task();
		break;
	}
	}
	return;
}

void gfx::present(Deferred onSwap)
{
	enqueue([onSwap = std::move(onSwap)]() {
#if defined(LE3D_HEAVY_RENDER_TOGGLE)
		if (g_bForceHeavyRender)
		{
			std::this_thread::sleep_for(stdch::milliseconds(20));
		}
#endif
		onSwap();
		if (contextImpl::g_context.swapCount > 0)
		{
			++contextImpl::g_context.framesRendered;
		}
	});
	++contextImpl::g_context.swapCount;
	g_renderer.present();
#if defined(LE3D_ASSERTS)
	u64 diff = contextImpl::g_context.swapCount - contextImpl::g_context.framesRendered;
	std::string msg("Invariant violated! diff: ");
	msg += std::to_string(diff);
	ASSERT_STR(diff <= 1, msg.data());
#endif
	// LOG_D("Ticked: %lu, Rendered: %lu", context::framesTicked(), context::framesRendered());
	return;
}
} // namespace le
