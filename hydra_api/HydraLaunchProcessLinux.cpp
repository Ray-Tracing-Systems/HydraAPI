//
// Created by vsan on 21.01.18.
//

#include "HydraLegacyUtils.h"

struct HydraProcessLauncher : IHydraNetPluginAPI
{
  HydraProcessLauncher(){}
  ~HydraProcessLauncher(){}

  bool hasConnection() const override;

  void runAllRenderProcesses(RenderProcessRunParams a_params, const std::vector<HydaRenderDevice>& a_devList) override;
  void stopAllRenderProcesses() override;
};


