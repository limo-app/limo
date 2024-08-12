#include "nexusmoddialog.h"
#include "tablepushbutton.h"
#include "ui_nexusmoddialog.h"
#include <QDebug>
#include <QLabel>
#include <QRegularExpression>
#include <QSettings>
#include <QSpacerItem>
#include <sstream>


NexusModDialog::NexusModDialog(QWidget* parent) : QDialog(parent), ui(new Ui::NexusModDialog)
{
  ui->setupUi(this);
  ui->files_widget->setLayout(new QVBoxLayout());
}

NexusModDialog::~NexusModDialog()
{
  delete ui;
}

void NexusModDialog::setupDialog(int app_id, int mod_id, const nexus::Page& page)
{
  app_id_ = app_id;
  mod_id_ = mod_id;
  page_ = page;
  QString description = page.mod.description.c_str();
  ui->description_box->setHtml(bbcodeToHtml(description));

  QString changelog;
  for(const auto& [version, changes] : page.changelog)
  {
    changelog.append(
      (R"(<span style="font-size:18px"><b>)" + version + R"(</b></span><ul>)").c_str());
    for(const auto& change : changes)
      changelog.append(("<li>" + change + "</li>").c_str());
    changelog.append("</ul><br />");
  }
  ui->changelog_box->setHtml(changelog);

  for(auto child : ui->files_widget->children())
    delete child;

  if(page.files.empty())
  {
    auto layout = ui->files_widget->layout();
    ui->files_widget->setLayout(new QVBoxLayout());
    delete layout;
    return;
  }

  const QString mod_link =
    std::format(
      "<span style=\"font-size:17px\"><b>"
      "<a href=\"https://nexusmods.com/{}/mods/{}\">Link To NexusMods Page</a></b></span>",
      page.mod.domain_name,
      page.mod.mod_id)
      .c_str();
  ui->link_label_desc->setText(mod_link);
  ui->link_label_changelog->setText(mod_link);
  ui->link_label_files->setText(
    std::format("<span style=\"font-size:17px\"><b>"
                "<a href=\"https://nexusmods.com/{}/mods/{}?tab=files\">Link To NexusMods "
                "Page</a></b></span>",
                page.mod.domain_name,
                page.mod.mod_id)
      .c_str());

  std::vector<nexus::File> files = page.files;
  std::sort(files.begin(),
            files.end(),
            [](nexus::File f1, nexus::File f2)
            {
              if(f1.category_id != f2.category_id)
                return f1.category_id < f2.category_id;
              return f1.name < f2.name;
            });
  auto base_layout = new QVBoxLayout();
  auto old_layout = ui->files_widget->layout();
  delete old_layout;
  ui->files_widget->setLayout(base_layout);
  long cur_cat_id = -1;
  QString cur_cat_name = "";
  for(const auto& file : files)
  {
    if(file.category_id != cur_cat_id)
    {
      cur_cat_id = file.category_id;
      cur_cat_name = file.category_name.empty() ? "Other" : file.category_name.c_str();
      auto label = new QLabel();
      label->setTextFormat(Qt::RichText);
      label->setText(R"(<span style="font-size:18px"><b>)" + cur_cat_name + " Files</b></span>");
      base_layout->addWidget(label);
    }
    auto frame = new QFrame();
    frame->setFrameShadow(QFrame::Plain);
    frame->setFrameShape(QFrame::Panel);
    auto frame_layout = new QVBoxLayout();
    frame->setLayout(frame_layout);
    QString mod_text;
    mod_text.append((R"(<span style="font-size:17px"><b>)" + file.name + "</b></span>").c_str());
    if(!file.version.empty())
      mod_text.append(("<br /><b>Version: " + file.version + "</b>").c_str());
    std::stringstream ss;
    ss << std::put_time(std::localtime(&file.uploaded_time), "%F %T");
    mod_text.append(("<br /><b>Upload Time: " + ss.str() + "</b>").c_str());
    QString size_string;
    long size = file.size_in_bytes;
    if(size < 1024)
      size_string = QString::number(size);
    else
    {
      long last_size = 0;
      int exp = 0;
      const std::vector<QString> units{ "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB" };
      while(size > 1024 && exp < units.size())
      {
        last_size = size;
        size /= 1024;
        exp++;
      }
      last_size /= 1.024;
      size_string = QString::number(size);
      const int first_digit = (last_size / 100) % 10;
      const int second_digit = (last_size / 10) % 10;
      if(first_digit != 0 || second_digit != 0)
        size_string += "." + QString::number(first_digit);
      if(second_digit != 0)
        size_string += QString::number(second_digit);
      size_string += " " + units[exp];
    }
    mod_text.append("<br /><b>Size: " + size_string + "</b><br />");
    if(file.external_virus_scan_url.empty())
      mod_text.append("<b>No external virus scan</b><br />");
    else
      mod_text.append(
        ("<b><a href=\"" + file.external_virus_scan_url + "\">Virus scan link</a><b><br />")
          .c_str());
    mod_text.append("<br /><br />" + bbcodeToHtml(file.description.c_str()) + "<br />");
    if(!file.changelog_html.empty())
      mod_text.append("<br /><b>Changes</b><br />" + bbcodeToHtml(file.changelog_html.c_str()) +
                      "<br />");
    auto text_label = new QLabel();
    text_label->setTextFormat(Qt::RichText);
    text_label->setText(mod_text);
    text_label->setWordWrap(true);
    text_label->setTextInteractionFlags(text_label->textInteractionFlags() |
                                        Qt::TextSelectableByMouse);
    frame_layout->addWidget(text_label);

    auto manual_link_label = new QLabel();
    manual_link_label->setTextFormat(Qt::RichText);
    manual_link_label->setText(
      std::format("<b><a href=\"https://nexusmods.com/{}/mods/{}?tab=files&file_id={}\">"
                  "Manual Download Link</a></b>",
                  page.mod.domain_name,
                  page.mod.mod_id,
                  file.file_id)
        .c_str());
    manual_link_label->setOpenExternalLinks(true);
    frame_layout->addWidget(manual_link_label);

    auto manager_link_label = new QLabel();
    manager_link_label->setTextFormat(Qt::RichText);
    manager_link_label->setText(
      std::format("<b><a href=\"https://nexusmods.com/{}/mods/{}?tab=files&file_id={}&nmm=1\">"
                  "Mod Manager Download Link</a></b>",
                  page.mod.domain_name,
                  page.mod.mod_id,
                  file.file_id)
        .c_str());
    manager_link_label->setOpenExternalLinks(true);
    frame_layout->addWidget(manager_link_label);
    QSettings settings(QCoreApplication::applicationName());
    settings.beginGroup("nexus");
    const bool is_premium = settings.value("info_is_premium", false).toBool();
    settings.endGroup();
    if(is_premium)
    {
      auto download_button = new TablePushButton(file.file_id, file.file_id);
      download_button->setText("Download");
      download_button->setIcon(QIcon::fromTheme("edit-download"));
      connect(
        download_button, &TablePushButton::clickedAt, this, &NexusModDialog::onDownloadClicked);

      frame_layout->addWidget(download_button);
    }
    frame_layout->addStretch();
    base_layout->addWidget(frame);
  }
  base_layout->addStretch();
}

QString NexusModDialog::bbcodeToHtml(const QString& bbcode)
{
  QString html = bbcode;
  html.remove(QChar(0xFEFF));
  html.remove("\ufeff");
  html.replace("\xa0", " ");
  html.remove("\n");

  std::vector<std::tuple<QString, QString, QString>> tokens = {
    { R"(\[center\])", R"(\[/center\])", R"(<center>\1</center>)" },
    { R"(\[b\])", R"(\[/b\])", R"(<b>\1</b>)" },
    { R"(\[i\])", R"(\[/i\])", R"(<i>\1</i>)" },
    { R"(\[u\])", R"(\[/u\])", R"(<u>\1</u>)" },
    { R"(\[s\])", R"(\[/s\])", R"(<s>\1</s>)" },
    { R"(\[url\])", R"(\[/url\])", R"(<a href="\1">\1</a>)" },
    { R"(\[url=(.*?)\])", R"(\[/url\])", R"(<a href="\1">\2</a>)" },
    { R"(\[youtube\])",
      R"(\[/youtube\])",
      R"(<a href="https://www.youtube.com/watch?v=\1">https://www.youtube.com/watch?v=\1</a>)" },
    // { R"(\[img\])", R"(\[/img\])", R"(<img src="\1" />)" },
    { R"(\[img\])", R"(\[/img\])", R"(<a href="\1"> [\1] </a>)" },
    { R"(\[quote\])", R"(\[/quote\])", R"(<blockquote>\1</blockquote>)" },
    { R"(\[quote=(.*?)\])", R"(\[/quote\])", R"(<blockquote><cite>\1</cite>\2</blockquote>)" },
    { R"(\[code\])", R"(\[/code\])", R"(<pre><code>\1</code></pre>)" },
    { R"(\[list\])", R"(\[/list\])", R"(<ul>\1</ul>)" },
    { R"(\[list=1\])", R"(\[/list\])", R"(<ol>\1</ol>)" },
    { R"(\[li\])", R"(\[/li\])", R"(<li>\1</li>)" },
    { R"(\[color=(.*?)\])", R"(\[/color\])", R"(<span style="color:\1">\2</span>)" },
    { R"(\[size=(.*?)\])", R"(\[/size\])", R"(<span style="font-size:\1">\2</span>)" },
    { R"(\[left\])", R"(\[/left\])", R"(<span align="left">\1</span>)" }
  };

  for(const auto& [begin, end, replace] : tokens)
  {
    while(html.contains(QRegularExpression(begin + R"((.*?))" + end)))
      html.replace(QRegularExpression(begin + "((?!" + begin + R"().*?))" + end), replace);
  }

  return html;
}

void NexusModDialog::onDownloadClicked(int file_id, int file_id_copy)
{
  emit modDownloadRequested(app_id_, mod_id_, file_id, page_.url.c_str());
}
