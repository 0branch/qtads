// This is copyrighted software. More information is at the end of this file.
#include "gameinfodialog.h"

#include "globals.h"
#include "htmlfmt.h"
#include "htmlrf.h"
#include "settings.h"
#include "sysframe.h"
#include "syswininput.h"
#include "ui_gameinfodialog.h"
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>

void QTadsGameInfoEnum::tads_enum_game_info(const char* const name, const char* const val)
{
    const auto valStr = QString::fromUtf8(val);
    const auto nameStr = QString::fromUtf8(name).toLower();
    const auto htmlValStr = valStr.toHtmlEscaped();

    if (nameStr == "name") {
        gameName = "<b><center><font size=\"+1\">" + htmlValStr + "</font></center></b><p>";
        plainGameName = valStr;
    } else if (nameStr == "headline") {
        headline = "<center>" + htmlValStr + "</center><p>";
    } else if (nameStr == "byline") {
        byLine = "<i><center>" + htmlValStr + "</center></i><p>";
    } else if (nameStr == "htmlbyline") {
        htmlByLine = "<i><center>" + valStr + "</center></i><p>";
    } else if (nameStr == "authoremail") {
        email = valStr;
    } else if (nameStr == "desc") {
        desc = htmlValStr;
        desc.replace("\\n", "<p>");
    } else if (nameStr == "htmldesc") {
        htmlDesc = valStr;
    } else if (nameStr == "version") {
        version = valStr;
    } else if (nameStr == "firstpublished") {
        published = valStr;
    } else if (nameStr == "releasedate") {
        date = valStr;
    } else if (nameStr == "language") {
        lang = valStr;
    } else if (nameStr == "series") {
        series = valStr;
    } else if (nameStr == "seriesnumber") {
        seriesNumber = valStr;
    } else if (nameStr == "genre") {
        genre = valStr;
    } else if (nameStr == "forgiveness") {
        forgiveness = valStr;
    } else if (nameStr == "licensetype") {
        license = valStr;
    } else if (nameStr == "copyingrules") {
        copyRules = valStr;
    } else if (nameStr == "ifid") {
        ifid = valStr;
    }
}

static void insertTableRow(QTableWidget& table, const QString& text1, const QString& text2)
{
    table.insertRow(table.rowCount());
    auto* item = new QTableWidgetItem(text1);
    item->setFlags(Qt::ItemIsEnabled);
    table.setItem(table.rowCount() - 1, 0, item);
    item = new QTableWidgetItem(text2);
    item->setFlags(Qt::ItemIsEnabled);
    table.setItem(table.rowCount() - 1, 1, item);
}

static auto loadCoverArtImage() -> QImage
{
    auto* const resFinder = qFrame->gameWindow()->get_formatter()->get_res_finder();

    // Look for a cover art resource. We try four different resource names.
    // The first two (PNG and JPG with a ".system/" prefix) are defined in the
    // current cover art standard.  The other two were defined in the older
    // standard which did not use a prefix.
    std::string coverArtResName;
    for (std::string path :
         {".system/CoverArt.png", ".system/CoverArt.jpg", "CoverArt.png", "CoverArt.jpg"})
    {
        if (resFinder->resfile_exists(path.c_str(), path.length())) {
            coverArtResName = std::move(path);
            break;
        }
    }
    if (coverArtResName.empty()) {
        return {};
    }

    CStringBuf strBuf;
    unsigned long offset;
    unsigned long size;
    resFinder->get_file_info(
        &strBuf, coverArtResName.c_str(), coverArtResName.length(), &offset, &size);

    // Open the file and seek to the specified position.
    QFile file(fnameToQStr(strBuf.get()));
    if (not file.open(QIODevice::ReadOnly)) {
        qWarning() << "ERROR: Can't open file" << file.fileName();
        return {};
    }
    if (not file.seek(offset)) {
        qWarning() << "ERROR: Can't seek in file" << file.fileName();
        return {};
    }

    // Load the image data.
    const auto data = file.read(size);
    file.close();
    if (data.isEmpty() or static_cast<unsigned long>(data.size()) < size) {
        qWarning() << "ERROR: Could not read" << size << "bytes from file" << file.fileName();
        return {};
    }
    QImage image;
    if (not image.loadFromData(data)) {
        qWarning() << "ERROR: Could not parse image data";
        return {};
    }

    // If we got here, all went well.  Return the image scaled to a
    // 200 pixels width if it's too large.  Otherwise, return it as-is.
    if (image.width() > 200) {
        const auto mode =
            qFrame->settings().useSmoothScaling ? Qt::SmoothTransformation : Qt::FastTransformation;
        return image.scaledToWidth(200, mode);
    } else {
        return image;
    }
}

GameInfoDialog::GameInfoDialog(const QByteArray& fname, QWidget* const parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
    , ui(std::make_unique<Ui::GameInfoDialog>())
{
    ui->setupUi(this);

    // Try to load the cover art.  If there is one, insert it into the text
    // browser as the "CoverArt" resource.
    const auto image = loadCoverArtImage();
    if (not image.isNull()) {
        ui->description->document()->addResource(
            QTextDocument::ImageResource, QUrl("CoverArt"), image);
        resize(width(), height() + image.height());
    }

    CTadsGameInfo info;
    info.read_from_file(fname.constData());
    QTadsGameInfoEnum cb;
    info.enum_values(&cb);

    // Fill out the description.
    QString tmp;
    if (not image.isNull()) {
        tmp += "<center><img src=\"CoverArt\"></center><p>";
    }
    tmp += cb.gameName + cb.headline + (cb.htmlByLine.isEmpty() ? cb.byLine : cb.htmlByLine)
        + (cb.htmlDesc.isEmpty() ? cb.desc : cb.htmlDesc);
    ui->description->setHtml(tmp);

    // Fill out the table.
    ui->table->setColumnCount(2);
    if (not cb.genre.isEmpty()) {
        insertTableRow(*ui->table, tr("Genre"), cb.genre);
    }

    if (not cb.version.isEmpty()) {
        insertTableRow(*ui->table, tr("Version"), cb.version);
    }

    if (not cb.forgiveness.isEmpty()) {
        insertTableRow(*ui->table, tr("Forgiveness"), cb.forgiveness);
    }

    if (not cb.series.isEmpty()) {
        insertTableRow(*ui->table, tr("Series"), cb.series);
    }

    if (not cb.seriesNumber.isEmpty()) {
        insertTableRow(*ui->table, tr("Series Number"), cb.seriesNumber);
    }

    if (not cb.date.isEmpty()) {
        insertTableRow(*ui->table, tr("Date"), cb.date);
    }

    if (not cb.published.isEmpty()) {
        insertTableRow(*ui->table, tr("First Published"), cb.published);
    }

    if (not cb.email.isEmpty()) {
        insertTableRow(*ui->table, tr("Author email"), cb.email);
    }

    if (not cb.lang.isEmpty()) {
        insertTableRow(*ui->table, tr("Language"), cb.lang);
    }

    if (not cb.license.isEmpty()) {
        insertTableRow(*ui->table, tr("License Type"), cb.license);
    }

    if (not cb.copyRules.isEmpty()) {
        insertTableRow(*ui->table, tr("Copying Rules"), cb.copyRules);
    }

    if (not cb.ifid.isEmpty()) {
        insertTableRow(*ui->table, tr("IFID"), cb.ifid);
    }

    ui->table->resizeColumnsToContents();
    ui->table->resizeRowsToContents();
    ui->table->setShowGrid(false);
    // There's no point in having the tableview widget be any higher than the
    // sum of all rows.  This will also make sure that the widget is not shown
    // at all in case we have zero rows.
    const auto margins = ui->table->contentsMargins();
    auto maxHeight = ui->table->rowCount() * ui->table->rowHeight(0);
    if (maxHeight > 0) {
        // Only add the margins to the maximum height if we're going to show
        // the table at all.
        maxHeight += margins.top() + margins.bottom();
        if (maxHeight < ui->table->minimumSizeHint().height()) {
            // Do not make it smaller than the minimum size hint, otherwise we'll
            // have a messed-up scrollbar.
            maxHeight = ui->table->minimumSizeHint().height();
        }
    }
    ui->table->setMaximumHeight(maxHeight);
}

GameInfoDialog::~GameInfoDialog() = default;

auto GameInfoDialog::gameHasMetaInfo(const QByteArray& fname) -> bool
{
    CTadsGameInfo info;
    return info.read_from_file(fname.constData());
}

auto GameInfoDialog::getMetaInfo(const QByteArray& fname) -> QTadsGameInfoEnum
{
    CTadsGameInfo info;
    QTadsGameInfoEnum cb;
    info.read_from_file(fname.constData());
    info.enum_values(&cb);
    return cb;
}

/*
    Copyright 2003-2020 Nikos Chantziaras <realnc@gmail.com>

    This file is part of QTads.

    QTads is free software: you can redistribute it and/or modify it under the
    terms of the GNU General Public License as published by the Free Software
    Foundation, either version 3 of the License, or (at your option) any later
    version.

    QTads is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
    details.

    You should have received a copy of the GNU General Public License along
    with QTads. If not, see <https://www.gnu.org/licenses/>.
*/
