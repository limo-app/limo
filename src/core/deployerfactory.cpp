#include "deployerfactory.h"
#include "casematchingdeployer.h"
#include "lootdeployer.h"


std::unique_ptr<Deployer> DeployerFactory::makeDeployer(const std::string& type,
                                                        const std::filesystem::path& source_path,
                                                        const std::filesystem::path& dest_path,
                                                        const std::string& name,
                                                        bool use_copy_deployment)
{
  if(type == SIMPLEDEPLOYER)
    return std::make_unique<Deployer>(source_path, dest_path, name, use_copy_deployment);
  else if(type == CASEMATCHINGDEPLOYER)
    return std::make_unique<CaseMatchingDeployer>(
      source_path, dest_path, name, use_copy_deployment);
  else if(type == LOOTDEPLOYER)
    return std::make_unique<LootDeployer>(source_path, dest_path, name);
  else
    throw std::runtime_error("Unknown deployer type \"" + type + "\"!");
}
