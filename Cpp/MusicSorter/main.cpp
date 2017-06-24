/*
    Copyright (C) 2017 Aseman Land
    http://aseman.co

    This project is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This project is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QStringList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QDebug>

#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QThread>

#include <taglib/fileref.h>

void start(const QString &collectionPath, const QString &musicsPath, QProgressBar *progressBar = Q_NULLPTR, QLabel *resLabel = Q_NULLPTR)
{
    QHash<QString, QPair<QString,QString> > artists;
    QStringList collections;
    if(!collectionPath.isEmpty())
        collections << collectionPath;

    foreach( const QString & clc, collections )
    {
        const QStringList & dirs = QDir(clc).entryList(QDir::Dirs|QDir::NoDotAndDotDot);
        foreach( const QString & dir, dirs )
        {
            artists.insert( dir.toLower(), QPair<QString,QString>(clc,dir) );
        }
    }

    QDir musics(musicsPath);
    const QStringList & files = musics.entryList(QStringList()<<"*.mp3",QDir::Files);
    if(progressBar)
        progressBar->setMaximum(files.count());

    int counter = 0;
    foreach(const QString &file, files)
    {
        QString newName = file;
        if(resLabel)
            resLabel->setText(file);

        TagLib::FileRef f( QString(musicsPath + "/" + file).toUtf8() );
        QString artist_name = f.tag()->artist().toCString();
        QString title = f.tag()->title().toCString();
        QString album_name = f.tag()->album().toCString();

        if(artist_name.isEmpty())
            artist_name = file.mid(0,file.indexOf("-")).trimmed();
        else
        if(!title.isEmpty())
            newName = artist_name + " - " + title + ".mp3";

        QString artist_dir = artists.value( artist_name.toLower() ).second;
        if( artist_dir.isEmpty() )
            artist_dir = artist_name;

        if(album_name.contains("single", Qt::CaseInsensitive) || album_name.isEmpty())
        {
            album_name = "Singles";
            QString artistTmp = artists.value( artist_name.toLower() ).second;
            if( !artistTmp.isEmpty() )
            {
                QString single_dir = artists.value( artist_name.toLower() ).first + "/" + artistTmp + "/" + album_name;
                if( !QFile::exists(single_dir) )
                    album_name = "Single";
            }
        }

        const QString & dir_path = QString(musicsPath) + "/" + artist_dir + "/" + album_name;

        QDir().mkpath(dir_path);
        qDebug() << file << newName;
        QFile(musicsPath+"/"+file).rename( dir_path + "/" + newName );

        if(progressBar)
        {
            progressBar->setValue(counter);
        }
        counter++;

        QCoreApplication::processEvents();
        QThread::msleep(20);
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDialog dialog;
    dialog.setMinimumWidth(350);

    QLabel *descLabel = new QLabel("<font color=\"#ff0000\">Musics will sort in the source path.</font><br />"
                                   "<b>Collection path</b> is not important. It just needed to get idea and samples "
                                   "from your sorted collection, to get better result.");

    QLabel *resLabel = new QLabel();
    resLabel->setWordWrap(true);
    resLabel->hide();

    descLabel->setWordWrap(true);

    QPushButton *startBtn = new QPushButton("Start");
    startBtn->setDisabled(true);

    QProgressBar *progressBar = new QProgressBar();
    progressBar->hide();

    /*! Source section !*/
    QLineEdit *srcLine = new QLineEdit();
    srcLine->setPlaceholderText("Unsorted musics");
    srcLine->setReadOnly(true);

    QPushButton *srcBtn = new QPushButton("Open");
    srcBtn->connect(srcBtn, &QPushButton::clicked, srcBtn, [&dialog, srcLine, startBtn, resLabel](){
        const QString &res = QFileDialog::getExistingDirectory(&dialog);
        if(!res.isEmpty())
        {
            srcLine->setText(res);
            startBtn->setEnabled(true);
            startBtn->setFocus();

            const QStringList & files = QDir(res).entryList(QStringList()<<"*.mp3",QDir::Files);
            resLabel->setText( QString("There are %1 unsorted musics in the source path.").arg(files.count()) );
            resLabel->show();
        }
    });

    QHBoxLayout *srcLayout = new QHBoxLayout();
    srcLayout->addWidget(srcLine);
    srcLayout->addWidget(srcBtn);


    /*! Destionation section !*/
    QLineEdit *clcLine = new QLineEdit();
    clcLine->setPlaceholderText("Collection path (not important)");
    clcLine->setReadOnly(true);

    QPushButton *clcBtn = new QPushButton("Open");
    clcBtn->connect(clcBtn, &QPushButton::clicked, clcBtn, [&dialog, clcLine](){
        const QString &res = QFileDialog::getExistingDirectory(&dialog);
        if(!res.isEmpty())
            clcLine->setText(res);
    });

    QHBoxLayout *clcLayout = new QHBoxLayout();
    clcLayout->addWidget(clcLine);
    clcLayout->addWidget(clcBtn);

    startBtn->connect(startBtn, &QPushButton::clicked, startBtn, [&dialog, srcLine, clcLine, progressBar, startBtn, resLabel]{
        startBtn->hide();
        progressBar->show();

        start(clcLine->text(), srcLine->text(), progressBar, resLabel);

        resLabel->setText("Finished...");
        startBtn->show();
        progressBar->hide();
    });

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->addWidget(descLabel);
    mainLayout->addLayout(srcLayout);
    mainLayout->addLayout(clcLayout);
    mainLayout->addWidget(resLabel);
    mainLayout->addWidget(startBtn);
    mainLayout->addWidget(progressBar);

    dialog.show();

    return app.exec();
}
