#include "deployerfactory.h"
#include "casematchingdeployer.h"
#include "lootdeployer.h"


std::unique_ptr<Deployer> DeployerFactory::makeDeployer(const std::string& type,
                                                        const std::filesystem::path& source_path,
                                                        const std::filesystem::path& dest_path,
                                                        const std::string& name,
                                                        Deployer::DeployMode deploy_mode)
{
  if(type == SIMPLEDEPLOYER)
    return std::make_unique<Deployer>(source_path, dest_path, name, deploy_mode);
  else if(type == CASEMATCHINGDEPLOYER)
    return std::make_unique<CaseMatchingDeployer>(
      source_path, dest_path, name, deploy_mode);
  else if(type == LOOTDEPLOYER)
    return std::make_unique<LootDeployer>(source_path, dest_path, name);
  else
    throw std::runtime_error("Unknown deployer type \"" + type + "\"!");
}
