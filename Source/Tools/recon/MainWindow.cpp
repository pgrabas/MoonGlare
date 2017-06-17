#include PCH_HEADER
#include <qobject.h>

#include "Recon.h"

#include "MainWindow.h"
#include "ui_MainWindow.h"
             
using namespace MoonGlare::Debug::InsiderApi;

//--------------------------------

MainWindow::MainWindow(std::shared_ptr<ReconData> recon)
    : QMainWindow(nullptr), recon(std::move(recon))
{
    m_Ui = std::make_unique<Ui::MainWindow>();
    m_Ui->setupUi(this);

    connect(m_Ui->pushButton, &QPushButton::clicked, [this]() {
        auto txt = m_Ui->lineEdit->text();
        //m_Ui->lineEdit->setText("");
        Send(txt, true);
    });

    connect(m_Ui->listWidget, &QListWidget::itemDoubleClicked, [this](QListWidgetItem*item) {
        Send(item->text(), false);
    });

    connect(m_Ui->buttonBox, &QDialogButtonBox::clicked, [this](QAbstractButton *item) {
        close();
    });

    m_Ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_Ui->listWidget, &QListWidget::customContextMenuRequested, [this](const QPoint & p) {
        auto item = m_Ui->listWidget->itemAt(p);
        if (item) {
            QMenu menu;
            menu.addAction(item->text())->setEnabled(false);
            menu.addAction("Remove", [this, item]() {
                RemoveAll(item->text());
                Save();
            });
            menu.exec(m_Ui->listWidget->mapToGlobal(p));
        }
    });

    try {
        Load();
    }
    catch (...) {
        //ignore
    }
}

MainWindow::~MainWindow() {
    Save();
    m_Ui.release();
}

void MainWindow::RemoveAll(const QString &text) {
    for (int i = 0; i < m_Ui->listWidget->count();) {
        if (m_Ui->listWidget->item(i)->text() == text) {
            m_Ui->listWidget->takeItem(i);
        }
        else {
            ++i;
        }
    }
}

void MainWindow::Send(const QString &text, bool addToHistory) {
    if (addToHistory) {
        RemoveAll(text);
        m_Ui->listWidget->insertItem(0, text);
        Save();
    }
    std::string txt = text.toUtf8().data();
    recon->Send(txt);
}

static const QString HistoryFileName = "ReconHistory.txt";

void MainWindow::Save() {
               
    QStringList SL;
    for (int i = 0; i < m_Ui->listWidget->count(); ++i) {
        SL.append(m_Ui->listWidget->item(i)->text());
    }

    QFile fOut(HistoryFileName);
    if (fOut.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream s(&fOut);
        for (int i = 0; i < m_Ui->listWidget->count(); ++i) {
            s << m_Ui->listWidget->item(i)->text() << '\n';
        }
    }
    else {
        return;
    }
}

void MainWindow::Load() {
    QFile fIn(HistoryFileName);
    if (fIn.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream sIn(&fIn);
    }
    else {
        return;

    }
    m_Ui->listWidget->clear();

    while (!fIn.atEnd()) {
        QString t = fIn.readLine().trimmed();
        if(!t.isEmpty())
            m_Ui->listWidget->addItem(t);
    }
}
