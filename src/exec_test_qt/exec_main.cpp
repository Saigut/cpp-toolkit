#include <iostream>
#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QStackedLayout>
#include <QScrollArea>
#include <QSplitter>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QToolBar>
#include <QStatusBar>
#include <QComboBox>
#include <QMessageBox>
#include <QtPlugin>


#ifdef Q_OS_MAC
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin)  // macOS 平台
#elif defined(Q_OS_WIN)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)  // Windows 平台
#elif defined(Q_OS_LINUX)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)  // Linux 平台
#else
#error "Unsupported platform!"
#endif


class MainWindow : public QMainWindow {
Q_OBJECT
public:
    MainWindow() {
        QTabWidget *tabWidget = new QTabWidget(this);

        // 保留所有布局和组件
        tabWidget->addTab(createVerticalLayoutTab(), "Vertical Layout");
        tabWidget->addTab(createHorizontalLayoutTab(), "Horizontal Layout");
        tabWidget->addTab(createGridLayoutTab(), "Grid Layout");
        tabWidget->addTab(createFormLayoutTab(), "Form Layout");
        tabWidget->addTab(createStackedLayoutTab(), "Stacked Layout");
        tabWidget->addTab(createSplitterTab(), "Splitter Layout");
        tabWidget->addTab(createScrollAreaTab(), "Scroll Area");
        tabWidget->addTab(createComplexLayoutTab(), "Complex Layout");

        // 新增信号与槽演示
        tabWidget->addTab(createSignalSlotTab(), "Signal & Slot Demo");

        setCentralWidget(tabWidget);
        setWindowTitle("Qt Features Example");
        resize(800, 600);

        // 添加工具栏和状态栏
        QToolBar *toolBar = addToolBar("Main Toolbar");
        toolBar->addAction("Action 1");
        toolBar->addAction("Action 2");
        toolBar->addAction("Quit", this, &QMainWindow::close);

        QStatusBar *statusBar = new QStatusBar(this);
        setStatusBar(statusBar);
        statusBar->showMessage("Ready");
    }

private:
    QWidget* createSignalSlotTab() {
        QWidget *tab = new QWidget;
        QVBoxLayout *layout = new QVBoxLayout(tab);

        QLabel *label = new QLabel("Choose an option:");
        QComboBox *comboBox = new QComboBox;
        comboBox->addItem("Option 1");
        comboBox->addItem("Option 2");
        comboBox->addItem("Option 3");

        QLabel *outputLabel = new QLabel("Your selection will appear here.");
        QPushButton *alertButton = new QPushButton("Show Alert");

        // 信号与槽：动态更新文本
        connect(comboBox, &QComboBox::currentTextChanged, outputLabel, &QLabel::setText);

        // 信号与槽：弹出消息框
        connect(alertButton, &QPushButton::clicked, [outputLabel]() {
            QMessageBox::information(nullptr, "Selection", "You selected: " + outputLabel->text());
        });

        layout->addWidget(label);
        layout->addWidget(comboBox);
        layout->addWidget(outputLabel);
        layout->addWidget(alertButton);

        return tab;
    }

    QWidget* createVerticalLayoutTab() {
        QWidget *tab = new QWidget;
        QVBoxLayout *layout = new QVBoxLayout(tab);

        layout->addWidget(new QLabel("This is a vertical layout example."));
        layout->addWidget(new QPushButton("Button 1"));
        layout->addWidget(new QPushButton("Button 2"));
        layout->addWidget(new QPushButton("Button 3"));

        return tab;
    }

    QWidget* createHorizontalLayoutTab() {
        QWidget *tab = new QWidget;
        QHBoxLayout *layout = new QHBoxLayout(tab);

        layout->addWidget(new QLabel("This is a horizontal layout example."));
        layout->addWidget(new QPushButton("Button A"));
        layout->addWidget(new QPushButton("Button B"));
        layout->addWidget(new QPushButton("Button C"));

        return tab;
    }

    QWidget* createGridLayoutTab() {
        QWidget *tab = new QWidget;
        QGridLayout *layout = new QGridLayout(tab);

        layout->addWidget(new QLabel("Top Left"), 0, 0);
        layout->addWidget(new QPushButton("Top Right"), 0, 1);
        layout->addWidget(new QLabel("Bottom Left"), 1, 0);
        layout->addWidget(new QPushButton("Bottom Right"), 1, 1);

        return tab;
    }

    QWidget* createFormLayoutTab() {
        QWidget *tab = new QWidget;
        QFormLayout *layout = new QFormLayout(tab);

        layout->addRow(new QLabel("Name:"), new QLineEdit());
        layout->addRow(new QLabel("Email:"), new QLineEdit());
        layout->addRow(new QLabel("Phone:"), new QLineEdit());
        layout->addRow(new QPushButton("Submit"), new QPushButton("Cancel"));

        return tab;
    }

    QWidget* createStackedLayoutTab() {
        QWidget *tab = new QWidget;
        QStackedLayout *layout = new QStackedLayout(tab);

        QLabel *label1 = new QLabel("Stack 1");
        QLabel *label2 = new QLabel("Stack 2");
        QLabel *label3 = new QLabel("Stack 3");

        layout->addWidget(label1);
        layout->addWidget(label2);
        layout->addWidget(label3);

        QPushButton *nextButton = new QPushButton("Next");
        QObject::connect(nextButton, &QPushButton::clicked, [layout]() {
            int index = (layout->currentIndex() + 1) % layout->count();
            layout->setCurrentIndex(index);
        });

        QVBoxLayout *mainLayout = new QVBoxLayout(tab);
        mainLayout->addLayout(layout);
        mainLayout->addWidget(nextButton);

        return tab;
    }

    QWidget* createSplitterTab() {
        QWidget *tab = new QWidget;
        QSplitter *splitter = new QSplitter(tab);

        splitter->addWidget(new QLabel("Left Pane"));
        splitter->addWidget(new QLabel("Right Pane"));

        QVBoxLayout *layout = new QVBoxLayout(tab);
        layout->addWidget(splitter);

        return tab;
    }

    QWidget* createScrollAreaTab() {
        QWidget *tab = new QWidget;
        QScrollArea *scrollArea = new QScrollArea(tab);

        QWidget *content = new QWidget;
        QVBoxLayout *layout = new QVBoxLayout(content);

        for (int i = 0; i < 20; ++i) {
            layout->addWidget(new QLabel(QString("Label %1").arg(i + 1)));
        }

        scrollArea->setWidget(content);
        scrollArea->setWidgetResizable(true);

        QVBoxLayout *mainLayout = new QVBoxLayout(tab);
        mainLayout->addWidget(scrollArea);

        return tab;
    }

    QWidget* createComplexLayoutTab() {
        QWidget *tab = new QWidget;

        QVBoxLayout *mainLayout = new QVBoxLayout(tab);

        // 添加工具栏
        QToolBar *toolBar = new QToolBar("Complex Toolbar");
        toolBar->addAction("New");
        toolBar->addAction("Open");
        toolBar->addAction("Save");

        mainLayout->addWidget(toolBar);

        // 添加中心区域
        QSplitter *splitter = new QSplitter;
        splitter->addWidget(new QListWidget);
        splitter->addWidget(new QLabel("Main Content"));

        mainLayout->addWidget(splitter);

        // 添加状态栏
        QLabel *statusBar = new QLabel("Status: Ready");
        mainLayout->addWidget(statusBar);

        return tab;
    }
};

int qt_test(int argc, const char *argv[]) {
    QApplication app(argc, const_cast<char**>(argv));

    app.setStyle("Fusion");

    // 设置调色板
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
    darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Highlight, QColor(142, 45, 197).lighter());
    app.setPalette(darkPalette);

    // 设置样式表
    app.setStyleSheet(R"(
        QPushButton {
            background-color: #61AFEF;
            color: white;
            border-radius: 5px;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #528CCB;
        }
        QLabel {
            color: white;
        }
    )");

    MainWindow window;
    window.show();

    return app.exec();
}

static int program_main(int argc, const char* argv[]) {
    return qt_test(argc, argv);
}

int main(int argc, const char* argv[]) {
    try {
        return program_main(argc, argv);
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return -1;
    }
}

#include "exec_main.moc"
