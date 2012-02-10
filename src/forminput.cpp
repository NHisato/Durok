#include "forminput.h"
#include "ui_forminput.h"

FormInput::FormInput(QString *winputFilesPath, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormInput)
{
    ui->setupUi(this);
    pinputFilesPath = winputFilesPath;
}

FormInput::~FormInput()
{
    delete ui;
}
//------------------------------------------------------------------------------
void FormInput::setTitle(QString wfileKind)
{
    fileKind = wfileKind;
    if(wfileKind == "CSV"){
        ui->groupBox->setTitle(tr("���̑��f�[�^�i�J���}��؂�j"));
    }
    else{
        ui->groupBox->setTitle(wfileKind + tr("�t�@�C��"));
    }
}
//------------------------------------------------------------------------------
QString FormInput::copyFiles(QString targetFolder)
{
    QString copyErrorPath = "";

    // Copy target files into the working folder.
    for (int ii = 0; ii < ui->listWidget->count(); ii++)
    {
        QString fpath = ui->listWidget->item(ii)->text();
        QFile *file = new QFile(fpath);
        QFileInfo *fileinfo = new QFileInfo(fpath);

        // Check result of copy function
        if(! file->copy(targetFolder + "/" + fileinfo->fileName())){
            copyErrorPath = fpath;
            break;
        }
    }
    return copyErrorPath;
}

//------------------------------------------------------------------------------
int FormInput::getListWidgetCount(){
    return ui->listWidget->count();
}

//------------------------------------------------------------------------------
QListWidget *FormInput::getListWidget(){
    return ui->listWidget;
}

//------------------------------------------------------------------------------
void FormInput::on_pushButton_Add_clicked()
{

    QFileDialog::Options options;
//        options |= QFileDialog::DontUseNativeDialog;
    QString selectedFilter;
    QString wmsg = fileKind + "�t�@�C����I�����Ă��������B";

    QStringList files = QFileDialog::getOpenFileNames(
                                this, tr(wmsg.toAscii()),
                                *pinputFilesPath,
                                tr("�S�Ẵt�@�C�� (*)"),
                                &selectedFilter,
                                options);
    if (files.count()) {
        getTargetFiles(ui->listWidget, ui->labelFileStatus, files);
    }

}
//------------------------------------------------------------------------------
void FormInput::on_pushButton_Delete_clicked()
{
    QStringList wlist;

    for (int ii = 0; ii < ui->listWidget->count(); ii++) {
        if (! ui->listWidget->item(ii)->isSelected()) {
            wlist.append(ui->listWidget->item(ii)->text());
        }
    }
    ui->listWidget->clear();
    ui->listWidget->addItems(wlist);

    rewriteFileStatus(ui->listWidget, ui->labelFileStatus);
}

//------------------------------------------------------------------------------
void FormInput::getTargetFiles(QListWidget *listWidget, QLineEdit *labelFileStatus, QStringList files)
{
    for (int ii = 0; ii < files.count(); ii++) {
        QFileInfo *fileinfo = new QFileInfo(files[ii]);
        if(fileinfo->isFile()){
            if(! getTargetFile(listWidget, files[ii], files.count() - ii - 1)){
                break;
            }
            *pinputFilesPath = fileinfo->dir().absolutePath();
        }
        else{
            QMessageBox::StandardButton reply;
            reply = QMessageBox::information(this, tr("�m�F"),
                     tr("�t�H���_���̂��ׂẴt�@�C����I�����܂���?") + "\n\n" + files[ii],
                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort);
            if (reply == QMessageBox::Yes){
                if(! getTargetFolder(listWidget, files[ii])){
                    break;
                }
                *pinputFilesPath = files[ii];
            }
            else if(reply == QMessageBox::No){
                continue;
            }
            else{
                break;
            }
        }
    }
    rewriteFileStatus(listWidget, labelFileStatus);
}

//------------------------------------------------------------------------------
bool FormInput::getTargetFolder(QListWidget *listWidget, QString folder)
{
    bool isContinue = true;

    QDir dir(folder);
    QStringList files = dir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable);
    for( int ii = 0; ii < files.count(); ii++){
        QFileInfo *fileinfo = new QFileInfo(folder + "/" + files[ii]);
        if(fileinfo->isFile()){
            if(!getTargetFile(listWidget, folder + "/" + files[ii], files.count() - ii - 1)){
                isContinue = false;
                break;
            }
        }
        else{
            QMessageBox::StandardButton reply;
            reply = QMessageBox::information(this, tr("�m�F"),
                     tr("�t�H���_���̂��ׂẴt�@�C����I�����܂���?") + "\n\n" + folder + "/" + files[ii],
                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort);
            if (reply == QMessageBox::Yes){
                if(! getTargetFolder(listWidget, folder + "/" + files[ii])){
                    isContinue = false;
                    break;
                }
            }
            else if (reply == QMessageBox::No){
                continue;
            }
            else{
                isContinue = false;
                break;
            }
        }
    }

    return isContinue;
}

//------------------------------------------------------------------------------
bool FormInput::getTargetFile(QListWidget *listWidget, QString fpath, int rest)
{
    bool isContinue = true;
    int dupIndex;

    dupIndex = isDup(listWidget, fpath);
    if (dupIndex != -1) {
        QMessageBox::StandardButton reply;
        if(rest > 0){
            reply = QMessageBox::information(this, tr("�m�F"),
                tr("�t�@�C�������d�����Ă��܂��B�u�������܂���?") + "\n\n" + listWidget->item(dupIndex)->text() + "\n   " + tr("to") + "\n" + fpath,
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort);
            if(reply == QMessageBox::Abort){
                isContinue = false;
            }
            else if(reply == QMessageBox::Yes){
                listWidget->item(dupIndex)->setText(fpath);
            }
        }
        else{
            reply = QMessageBox::information(this, tr("�m�F"),
                tr("�t�@�C�������d�����Ă��܂��B�u�������܂���?") + "\n\n" + listWidget->item(dupIndex)->text() + "\n   " + tr("to") + "\n" + fpath,
                QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes){
                listWidget->item(dupIndex)->setText(fpath);
            }
        }
    }
    else{
        listWidget->addItem(fpath);
    }
    return isContinue;
}

//------------------------------------------------------------------------------
void FormInput::rewriteFileStatus(QListWidget *listWidget, QLineEdit *labelFileStatus)
{
    int fileCount = 0;
    qint64 totalFileSize = 0;
    qint64 tera = Q_INT64_C(1099511627776);
    qint64 peta = Q_INT64_C(1125899906842624);

    for (int ii = 0; ii < listWidget->count(); ii++)
    {
        QString fpath = listWidget->item(ii)->text();
        QFile *file = new QFile(fpath);
        totalFileSize += file->size();
        fileCount++;
    }
    QString fileSizeString = "";
    if (totalFileSize == 0)
    {
        fileSizeString = "0";
    }
    else
    {
        fileSizeString = QString("%L2").arg(totalFileSize);
    }
    QString aboutFileSizeString = "";
    int aboutFileSize;
    if(totalFileSize < 1024){
        aboutFileSizeString = "";
    }
    else if(totalFileSize < 1024 * 1024){
        aboutFileSize = (int)(totalFileSize / 1024);
        aboutFileSizeString = " (" + tr("��") + QString("%1").arg(aboutFileSize) + tr("���޲�") + ")";
    }
    else if (totalFileSize < 1073741824) // �P�M�K 1024 * 1024 * 1024
    {
        aboutFileSize = (int)(totalFileSize / (1024 * 1024));
        aboutFileSizeString = " (" + tr("��") + QString("%1").arg(aboutFileSize) + tr("Ҷ��޲�") + ")";
    }
    else if (totalFileSize < tera) // �P�e�� 1024 * 1024 * 1024 * 1024
    {
        aboutFileSize = (int)(totalFileSize / (1073741824));
        aboutFileSizeString = " (" + tr("��") + QString("%1").arg(aboutFileSize) + tr("�޶��޲�") + ")";
    }
    else if (totalFileSize < peta) // �P�y�^ 1024 * 1024 * 1024 * 1024 * 1024
    {
        aboutFileSize = (int)(totalFileSize / (tera));
        aboutFileSizeString = " (" + tr("��") + QString("%1").arg(aboutFileSize) + tr("���޲�") + ")";
    }

    labelFileStatus->setText(tr("̧�ِ� : ") + QString("%1").arg(fileCount) + "  " + tr("���v̧�ٻ��� : ") + fileSizeString + aboutFileSizeString);
}

//------------------------------------------------------------------------------
int FormInput::isDup(QListWidget *listWidget, QString fpath)
{
    int index = -1;

    QFileInfo *fileinfo = new QFileInfo(fpath);
    QString fname = fileinfo->fileName();
    for (int ii = 0; ii < listWidget->count(); ii++) {
        if(listWidget->item(ii)->text() == fpath) {
            index = ii;
            break;
        }
        QFileInfo fi(listWidget->item(ii)->text());
        if(fname == fi.fileName()){
            index = ii;
            break;
        }
    }
    return index;
}
