#include "gui.hpp"
#include "handlers.hpp"

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QIcon>
#include <QProcess>

int run_gui(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("Lumen");
    app.setApplicationDisplayName("Lumen");

    // =========================
    // ГЛАВНОЕ ОКНО
    // =========================

    QMainWindow window;

    QWidget *central = new QWidget;
    window.setCentralWidget(central);

    QHBoxLayout *mainLayout = new QHBoxLayout(central);

    // =========================
    // ЛЕВАЯ ПАНЕЛЬ
    // =========================

    QWidget *sidebar = new QWidget;
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);

    // =========================
    // SEARCH BAR
    // =========================

    QLineEdit *search = new QLineEdit;

    search->setPlaceholderText("Search applications...");

    sidebarLayout->addWidget(search);

    // =========================
    // СПИСОК ПРИЛОЖЕНИЙ
    // =========================

    QListWidget *appList = new QListWidget;

    sidebarLayout->addWidget(appList);

    // =========================
    // ПРАВАЯ ПАНЕЛЬ
    // =========================

    QWidget *rightPanel = new QWidget;

    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);

    // =========================
    // ДОБАВЛЯЕМ ПАНЕЛИ
    // =========================

    mainLayout->addWidget(sidebar, 1);
    mainLayout->addWidget(rightPanel, 2);

    // =========================
    // ЗАГРУЖАЕМ ПРИЛОЖЕНИЯ
    // =========================

    load_app();

    for (const auto& application : applications)
    {
        QListWidgetItem *item = new QListWidgetItem;

        item->setText(QString::fromStdString(application.name));

        // Иконка
        QIcon icon = QIcon::fromTheme(
            QString::fromStdString(application.icon)
        );

        if (!icon.isNull())
        {
            item->setIcon(icon);
        }

        appList->addItem(item);
    }

    // =========================
    // КЛИК ПО ПРИЛОЖЕНИЮ
    // =========================

    QObject::connect(
        appList,
        &QListWidget::itemDoubleClicked,
        [&](QListWidgetItem *item)
        {
            QString name = item->text();

            window.close();

            open_app(name.toStdString());

        }
    );

    // =========================
    // ПОИСК
    // =========================

    QObject::connect(
        search,
        &QLineEdit::textChanged,
        [&](const QString &text)
        {
            QString query = text.toLower();

            for (int i = 0; i < appList->count(); ++i)
            {
                QListWidgetItem *item = appList->item(i);

                bool match =
                    item->text()
                        .toLower()
                        .contains(query);

                item->setHidden(!match);
            }
        }
    );

    // =========================
    // СТИЛИ
    // =========================

    window.setStyleSheet(R"(
        QMainWindow,
        QWidget {
            background-color: #000000;
            color: #ffffff;
        }

        QLineEdit {
            background-color: #000000;
            border: 1px solid #333333;
            padding: 10px;
            border-radius: 8px;
            color: #ffffff;
        }

        QLineEdit:focus {
            border: 1px solid #555555;
        }

        QListWidget {
            background-color: #000000;
            border: none;
            outline: none;
            color: #ffffff;
        }

        QListWidget::item {
            padding: 10px;
            border-radius: 8px;
        }

        QListWidget::item:hover {
            background-color: #151515;
        }

        QListWidget::item:selected {
            background-color: #202020;
        }
    )");

    // =========================
    // ОКНО
    // =========================

    window.setWindowTitle("Lumen");
    window.resize(800, 500);

    window.show();

    return app.exec();
}