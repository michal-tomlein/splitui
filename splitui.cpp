/******************************************************************************
 *                                   SplitUI                                  *
 * -------------------------------------------------------------------------- *
 * Version 0.1                                                                *
 * -------------------------------------------------------------------------- *
 * A tool for converting a Qt UI file into a header and a cpp file.           *
 * -------------------------------------------------------------------------- *
 * This programme is distributed under the terms of the GPL v2.               *
 * -------------------------------------------------------------------------- *
 * The programme is provided AS IS with ABSOLUTELY NO WARRANTY OF ANY KIND,   *
 * in a hope that it will be useful.                                          *
 ******************************************************************************/

#include <QApplication>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QProcess>

#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	
	if (app.arguments().count() <= 1) {
		cout << "Usage: splitui <class_name>.ui" << endl;
	} else {
		QFileInfo file_info(app.arguments().at(1));
		if (!file_info.exists())
			{ cout << "Invalid argument: " << QByteArray().append(app.arguments().at(1)).constData() << endl; return 0; }
	}
	QFileInfo ui_info(app.arguments().at(1));
	QStringList arguments; arguments << app.arguments().at(1) << "-o" << QString("ui_%1.h").arg(ui_info.completeBaseName());
	QProcess *uic = new QProcess;
	if (uic->execute("uic", arguments) != 0) { cout << "Error" << endl; return 0; }
	QString uih = QString("%1/ui_%2.h").arg(ui_info.absolutePath()).arg(ui_info.completeBaseName());
	QFile src_file(uih);
	if (!src_file.open(QFile::ReadOnly | QFile::Text))
		{ cout << "Error opening file " << QByteArray().append(uih).constData() << endl; return 0; }
	QFileInfo file_info(uih);
	QTextStream src(&src_file);
	QString header; QTextStream tmp(&header);
	QString buffer; QString class_name; QString widget_type;
	do {
		if (src.atEnd()) { cout << "Error" << endl; return 0; }
		buffer = src.readLine();
		tmp << buffer << endl;
	} while (!buffer.contains("class Ui_"));
	buffer.remove("class Ui_");
	class_name = buffer;
	buffer = src.readLine();
	do {
		tmp << buffer << endl;
		if (src.atEnd()) { cout << "Error" << endl; return 0; }
		buffer = src.readLine();
	} while (!buffer.contains("void setupUi("));
	if (buffer.contains(";")) { return 0; }
	QFile cpp_file(QString("%1/%2.cpp").arg(file_info.absolutePath()).arg(file_info.completeBaseName()));
	if (!cpp_file.open(QFile::WriteOnly | QFile::Text))
		{ cout << "Error creating file " << QByteArray().append(cpp_file.fileName()).constData() << endl; return 0; }
	QTextStream cpp(&cpp_file);
	widget_type = buffer;
	widget_type.remove("void setupUi(");
	widget_type = widget_type.split(" ", QString::SkipEmptyParts).at(0);
	tmp << QString("    void setupUi(%1 *);").arg(widget_type) << endl;
	cpp << QString("#include \"%1\"").arg(file_info.fileName()) << endl << endl;
	cpp << QString("    void Ui_%1::setupUi(%2 *%3)").arg(class_name).arg(widget_type).arg(class_name) << endl;
	buffer = src.readLine();
	do {
		cpp << buffer << endl;
		if (src.atEnd()) { cout << "Error" << endl; return 0; }
		buffer = src.readLine();
	} while (!buffer.contains("void retranslateUi("));
	tmp << QString("    void retranslateUi(%1 *);").arg(widget_type) << endl;
	cpp << QString("    void Ui_%1::retranslateUi(%2 *%3)").arg(class_name).arg(widget_type).arg(class_name) << endl;
	do {
		if (src.atEnd()) { cout << "Error" << endl; return 0; }
		buffer = src.readLine();
		cpp << buffer << endl;
	} while (!buffer.contains("} // retranslateUi"));
	do {
		buffer = src.readLine();
		tmp << buffer << endl;
	} while (!src.atEnd());
	QFile h_file(file_info.absoluteFilePath());
	if (!h_file.open(QFile::WriteOnly | QFile::Text))
		{ cout << "Error writing file " << QByteArray().append(file_info.absoluteFilePath()).constData() << endl; return 0; }
	QTextStream h(&h_file);
	h << header << endl;
	
	return 0;
}
