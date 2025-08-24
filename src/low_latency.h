#pragma once

#include "low_latency_tech/low_latency_tech.h"

#include <dxgi.h>
#if _MSC_VER
#include <d3d12.h>
#else
#include "../external/d3d12.h"
#endif

#include "util.h"
#include <optional>

#define FRAME_REPORTS_BUFFER_SIZE 70
#define NVAPI_BUFFER_SIZE 64

struct FrameReport {
    uint64_t frameID;
    uint64_t inputSampleTime;
    uint64_t simStartTime;
    uint64_t simEndTime;
    uint64_t renderSubmitStartTime;
    uint64_t renderSubmitEndTime;
    uint64_t presentStartTime;
    uint64_t presentEndTime;
    uint64_t driverStartTime;
    uint64_t driverEndTime;
    uint64_t osRenderQueueStartTime;
    uint64_t osRenderQueueEndTime;
    uint64_t gpuRenderStartTime;
    uint64_t gpuRenderEndTime;
    uint32_t gpuActiveRenderTimeUs;
    uint32_t gpuFrameTimeUs;
    uint8_t rsvd[120];
};

class LowLatency {
private:
    LowLatencyTech* currently_active_tech; // TODO: mutex the access???
    FrameReport frame_reports[FRAME_REPORTS_BUFFER_SIZE]{};
    std::optional<bool> forced_fg;
    bool fg;
    uint32_t delay_deinit = 0;

    // Once set, it will be used for all init attempts
    void* forced_low_latency_context = nullptr;
    Mode forced_low_latency_tech = Mode::LatencyFlex;

    void update_effective_fg_state();
    void update_enabled_override();

    // D3D
    bool update_low_latency_tech(IUnknown* pDevice);
    void get_latency_result(NV_LATENCY_RESULT_PARAMS* pGetLatencyParams);
    void add_marker_to_report(NV_LATENCY_MARKER_PARAMS *pSetLatencyMarkerParams);
    
    // Vulkan
    bool update_low_latency_tech(HANDLE vkDevice);
    void get_latency_result(NV_VULKAN_LATENCY_RESULT_PARAMS *pGetLatencyParams);
    void add_marker_to_report(NV_VULKAN_LATENCY_MARKER_PARAMS *pSetLatencyMarkerParams);

public:
    LowLatency() = default;
    ~LowLatency() { deinit_current_tech(); };

    bool deinit_current_tech();
    void set_forced_fg(std::optional<bool> forced_fg) { this->forced_fg = forced_fg; };
    void set_fg_type(bool interpolated, uint64_t frame_id) {
        if (currently_active_tech)
            currently_active_tech->set_fg_type(interpolated, frame_id); 
    }
    bool get_low_latency_context(void** low_latency_context, Mode* low_latency_tech);
    bool set_low_latency_context(void* low_latency_context, Mode low_latency_tech);

    // D3D
    NvAPI_Status Sleep(IUnknown* pDevice);
    NvAPI_Status SetSleepMode(IUnknown* pDevice, NV_SET_SLEEP_MODE_PARAMS* pSetSleepModeParams);
    NvAPI_Status GetSleepStatus(IUnknown* pDevice, NV_GET_SLEEP_STATUS_PARAMS* pGetSleepStatusParams);
    NvAPI_Status SetLatencyMarker(IUnknown *pDev, NV_LATENCY_MARKER_PARAMS *pSetLatencyMarkerParams);
    NvAPI_Status SetAsyncFrameMarker(ID3D12CommandQueue *pCommandQueue, NV_ASYNC_FRAME_MARKER_PARAMS *pSetAsyncFrameMarkerParams);
    NvAPI_Status GetLatency(IUnknown* pDev, NV_LATENCY_RESULT_PARAMS* pGetLatencyParams);

    // Vulkan
    NvAPI_Status Sleep(HANDLE vkDevice);
    NvAPI_Status SetSleepMode(HANDLE vkDevice, NV_VULKAN_SET_SLEEP_MODE_PARAMS* pSetSleepModeParams);
    NvAPI_Status GetSleepStatus(HANDLE vkDevice, NV_VULKAN_GET_SLEEP_STATUS_PARAMS* pGetSleepStatusParams);
    NvAPI_Status SetLatencyMarker(HANDLE vkDevice, NV_VULKAN_LATENCY_MARKER_PARAMS *pSetLatencyMarkerParams);
    NvAPI_Status GetLatency(HANDLE vkDevice, NV_VULKAN_LATENCY_RESULT_PARAMS* pGetLatencyParams);
};