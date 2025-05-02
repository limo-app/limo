#include "changelogdialog.h"
#include "core/consts.h"
#include "core/log.h"
#include "ui_changelogdialog.h"
#include <QScrollBar>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <json/json.h>

namespace sfs = std::filesystem;


ChangelogDialog::ChangelogDialog(bool is_flatpak, QWidget* parent) :
  is_flatpak_(is_flatpak), QDialog(parent), ui(new Ui::ChangelogDialog)
{
  ui->setupUi(this);
  setWindowTitle("Changelog");
  sfs::path path(is_flatpak ? "/app" : APP_INSTALL_PREFIX);
  path /= sfs::path("share") / "limo";
  if(!is_flatpak && sfs::exists("install_files"))
    path = "install_files";
  path /= "changelogs.json";
  Log::debug(std::format("Changelog path: '{}'", path.string()));
  if(!sfs::exists(path))
  {
    Log::error(std::format("Could not find changelog file at '{}'.", path.string()));
    return;
  }
  std::ifstream file(path, std::fstream::binary);
  if(!file.is_open())
  {
    Log::error(std::format("Failed to open changelog file at '{}'.", path.string()));
    return;
  }
  Json::Value json;
  file >> json;
  file.close();
  for(int i = 0; i < json["versions"].size(); i++)
    versions_.emplace_back(json["versions"][i]);
  std::sort(versions_.begin(), versions_.end());

  ui->version_box->blockSignals(true);
  for(const auto& version : versions_)
    ui->version_box->addItem(version.versionAndDateString().c_str());
  ui->version_box->setCurrentIndex(ui->version_box->count() - 1);
  ui->version_box->blockSignals(false);
  on_version_box_currentIndexChanged(ui->version_box->currentIndex());
  has_changes_ = true;
}

ChangelogDialog::~ChangelogDialog()
{
  delete ui;
}

void ChangelogDialog::on_version_box_currentIndexChanged(int index)
{
  ui->changelog->clear();
  if(index == -1)
    return;
  for(int i = index; i < versions_.size(); i++)
  {
    ChangelogEntry::ChangeType prev_type = ChangelogEntry::no_type;
    QString paragraph = std::format("<h2>Version {} - {}</h2><h3></h3>",
                                    versions_[i].getVersion(),
                                    versions_[i].getTitle())
                          .c_str();
    for(const auto& change : versions_[i].getChanges())
    {
      if(prev_type != change.getType())
      {
        ui->changelog->append(paragraph);
        paragraph.clear();

        if(change.getType() == ChangelogEntry::new_feature)
          ui->changelog->append("<h3>New Features</h3>");
        else if(change.getType() == ChangelogEntry::change)
          ui->changelog->append("<h3>Changes</h3>");
        else if(change.getType() == ChangelogEntry::fix)
          ui->changelog->append("<h3>Fixes</h3>");
        prev_type = change.getType();
      }

      paragraph.append(std::format(" - {}", change.getShortDescription()).c_str());
      if(change.getIssue() != -1 || change.getPullRequest() != -1)
        paragraph.append(" (");
      if(change.getIssue() != -1)
      {
        paragraph.append(
          std::format(
            "addresses <a href=\"https://github.com/limo-app/limo/issues/{0}\">issue {0}</a>",
            change.getIssue())
            .c_str());
        if(change.getPullRequest() != -1)
          paragraph.append(", ");
      }
      if(change.getPullRequest() != -1)
        paragraph.append(
          std::format(
            "implemented by <a href=\"https://github.com/limo-app/limo/pull/{0}\">PR {0}</a>",
            change.getPullRequest())
            .c_str());
      if(change.getIssue() != -1 || change.getPullRequest() != -1)
        paragraph.append(")");
      paragraph.append("<br />");
    }
    ui->changelog->append(paragraph);
  }
  ui->changelog->verticalScrollBar()->setValue(0);
}

bool ChangelogDialog::hasChanges() const
{
  return has_changes_;
}
