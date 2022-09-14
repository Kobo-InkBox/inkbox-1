#include "todo.h"
#include "ui_todo.h"

#include "functions.h"

todo::todo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::todo)
{
    ui->setupUi(this);
    this->setStyleSheet(readFile("/mnt/onboard/.adds/inkbox/eink.qss"));
    this->setFont(QFont("u001"));
    ui->listWidget->setFont(QFont("u001"));
    ui->itemsListWidget->setFont(QFont("u001"));

    ui->deleteBtn->setEnabled(false);
    ui->setupBtn->setEnabled(false);

    ui->closeBtn->setProperty("type", "borderless");
    ui->newBtn->setProperty("type", "borderless");
    ui->deleteBtn->setProperty("type", "borderless");
    ui->setupBtn->setProperty("type", "borderless");
    ui->deleteBtn->setStyleSheet("padding: 10px");
    ui->setupBtn->setStyleSheet("padding: 10px");
    ui->closeBtn->setIcon(QIcon(":/resources/close.png"));
    ui->newBtn->setIcon(QIcon(":/resources/new.png"));
    ui->deleteBtn->setIcon(QIcon(":/resources/x-circle.png"));
    ui->setupBtn->setIcon(QIcon(":/resources/arrow-right.png"));
    ui->listWidget->setStyleSheet("font-size: 10pt");
    ui->listWidget->setWordWrap(true);
    ui->itemsListWidget->setStyleSheet("font-size: 10pt");
    ui->itemsListWidget->setWordWrap(true);

    refreshList();
}

todo::~todo()
{
    delete ui;
}

void todo::on_closeBtn_clicked()
{
    if(currentView == currentView::home) {
        this->close();
    }
    else {
        saveCurrentList();
        ui->stackedWidget->setCurrentIndex(0);
        ui->closeBtn->setIcon(QIcon(":/resources/close.png"));
        ui->deleteBtn->setEnabled(true);
        ui->setupBtn->setEnabled(true);
        currentView = currentView::home;
    }
}


void todo::on_newBtn_clicked()
{
    global::keyboard::embed = false;
    virtualkeyboard * virtualKeyboardWidget = new virtualkeyboard(this);
    virtualKeyboardWidget->setAttribute(Qt::WA_DeleteOnClose);
    if(currentView == currentView::home) {
        QObject::connect(virtualKeyboardWidget, &virtualkeyboard::enterBtnPressed, this, &todo::createNewList);
        ui->statusLabel->setText("Enter the list's name");
    }
    else {
        QObject::connect(virtualKeyboardWidget, &virtualkeyboard::enterBtnPressed, this, &todo::addItem);
        ui->statusLabel->setText("Enter the item's name");
    }
    virtualKeyboardWidget->show();
}

void todo::createNewList(QString listName) {
    log("Initializing new To-Do list with name '" + listName + "'", className);
    QJsonDocument document;
    QJsonArray mainArray;
    QJsonArray listArray;
    QJsonObject object;
    if(QFile::exists(global::localLibrary::todoDatabasePath)) {
        document = readTodoDatabase();
        object = document.object();
        mainArray = object["List"].toArray();
    }
    listArray.append(listName);

    mainArray.push_back(listArray);
    object["List"] = mainArray;

    document.setObject(object);
    writeTodoDatabase(document);

    ui->statusLabel->setText("Select or create a new list");
    refreshList();
}

void todo::addItem(QString itemName) {
    log("Adding item with name '" + itemName + "' to current list", className);
    // Accessing the current list's items array
    QJsonDocument document = readTodoDatabase();
    QJsonObject object = document.object();
    QJsonArray mainArray = object["List"].toArray();
    QJsonArray listArray = mainArray.at(listIndex).toArray();
    QJsonArray itemArray;

    // Item name
    itemArray.insert(0, itemName);
    // Check state (always set to false)
    itemArray.insert(1, false);

    // Adding item array to list array
    listArray.push_back(itemArray);
    mainArray.replace(listIndex, listArray);
    object["List"] = mainArray;

    document.setObject(object);
    writeTodoDatabase(document);

    setupList(listArray.at(0).toString());
}

void todo::refreshList() {
    if(QFile::exists(global::localLibrary::todoDatabasePath)) {
        ui->listWidget->clear();
        QJsonArray array = readTodoDatabase().object()["List"].toArray();
        for(int i = 0; i < array.count(); i++) {
            QString name = array.at(i).toArray().at(0).toString();
            if(!name.isEmpty()) {
                ui->listWidget->addItem(name);
            }
        }
    }
}

void todo::setupList(QString listName) {
    log("Setting up list with name '" + listName + "'", className);
    QJsonArray array = readTodoDatabase().object()["List"].toArray();
    // Iterate through the To-Do lists array to find the list's array index
    for(int i = 0; i < array.count(); i++) {
        QString name = array.at(i).toArray().at(0).toString();
        if(name == listName) {
            listIndex = i;
            break;
        }
    }
    log("List index is " + QString::number(listIndex), className);

    ui->stackedWidget->setCurrentIndex(1);
    currentView = currentView::list;
    // Iterate through the selected list's array to find item arrays
    int count = array.at(listIndex).toArray().count();
    log("List's items count is " + QString::number(count - 1), className);
    // Starting at index 1 because 0 represents the list's name
    ui->itemsListWidget->clear();
    for(int i = 1; i < count; i++) {
        QJsonArray itemArray = array.at(listIndex).toArray().at(i).toArray();
        QListWidgetItem * item = new QListWidgetItem();
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        // Set the item's name
        item->setText(array.at(listIndex).toArray().at(i).toArray().at(0).toString());
        // Is the item checked?
        if(itemArray.at(1).toBool() == true) {
            item->setCheckState(Qt::Checked);
        }
        else {
            item->setCheckState(Qt::Unchecked);
        }
        ui->itemsListWidget->addItem(item);
    }
    ui->closeBtn->setIcon(QIcon(":/resources/check.png"));
    ui->statusLabel->setText("Select or create a new item");
}

void todo::on_setupBtn_clicked()
{
    ui->deleteBtn->setEnabled(false);
    ui->setupBtn->setEnabled(false);
    setupList(ui->listWidget->currentItem()->text());
}


void todo::on_listWidget_itemClicked(QListWidgetItem *item)
{
    ui->deleteBtn->setEnabled(true);
    ui->setupBtn->setEnabled(true);
}

void todo::saveCurrentList() {
    QJsonDocument document = readTodoDatabase();
    QJsonObject object = document.object();
    QJsonArray mainArray = object["List"].toArray();
    QJsonArray listArray = mainArray.at(listIndex).toArray();
    for(int i = 1; i < ui->itemsListWidget->count() + 1; i++) {
        QJsonArray itemArray = listArray.at(i).toArray();
        if(ui->itemsListWidget->item(i - 1)->checkState() == Qt::Checked) {
            itemArray.replace(1, true);
        }
        else {
            itemArray.replace(1, false);
        }
        listArray.replace(i, itemArray);
    }

    // Adding item array to list array
    mainArray.replace(listIndex, listArray);
    object["List"] = mainArray;

    document.setObject(object);
    writeTodoDatabase(document);
}

void todo::on_deleteBtn_clicked()
{

}