/******************************************************************************
 *                                   SplitUI                                  *
 * -------------------------------------------------------------------------- *
 * Version 0.2                                                                *
 * -------------------------------------------------------------------------- *
 * A tool for converting a Qt UI file into a header and a cpp file.           *
 * -------------------------------------------------------------------------- *
 * This programme is distributed under the terms of the GPL v2.               *
 * -------------------------------------------------------------------------- *
 * The programme is provided AS IS with ABSOLUTELY NO WARRANTY OF ANY KIND,   *
 * in a hope that it will be useful.                                          *
 ******************************************************************************/

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QProcess>

#include <iostream>

using namespace std;

int split(QString file_name)
{
	QFileInfo ui_info(file_name);
	if (!ui_info.exists())
		{ cout << "Invalid argument: " << QByteArray().append(file_name).constData() << endl; return -1; }
	QStringList arguments; arguments << file_name << "-o" << QString("%1/ui_%2.h").arg(ui_info.absolutePath()).arg(ui_info.completeBaseName());
	QProcess *uic = new QProcess;
	if (uic->execute("uic", arguments) != 0) { cout << "Error" << endl; return -1; }
	QString uih = QString("%1/ui_%2.h").arg(ui_info.absolutePath()).arg(ui_info.completeBaseName());
	QFile src_file(uih);
	if (!src_file.open(QFile::ReadOnly | QFile::Text))
		{ cout << "Error opening file " << QByteArray().append(uih).constData() << endl; return -1; }
	cout << QByteArray().append(ui_info.fileName()).constData() << " > uic successful" << endl;
	QFileInfo file_info(uih);
	QTextStream src(&src_file);
	QString header; QTextStream tmp(&header);
	QString buffer; QString class_name; QString widget_type;
	do {
		if (src.atEnd()) { cout << "Error" << endl; return -1; }
		buffer = src.readLine();
		tmp << buffer << endl;
	} while (!buffer.contains("class Ui_"));
	buffer.remove("class Ui_");
	class_name = buffer;
	buffer = src.readLine();
	do {
		tmp << buffer << endl;
		if (src.atEnd()) { cout << "Error" << endl; return -1; }
		buffer = src.readLine();
	} while (!buffer.contains("void setupUi("));
	if (buffer.contains(";")) { return 0; }
	QFile cpp_file(QString("%1/%2.cpp").arg(file_info.absolutePath()).arg(file_info.completeBaseName()));
	if (!cpp_file.open(QFile::WriteOnly | QFile::Text))
		{ cout << "Error creating file " << QByteArray().append(cpp_file.fileName()).constData() << endl; return -1; }
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
		if (src.atEnd()) { cout << "Error" << endl; return -1; }
		buffer = src.readLine();
	} while (!buffer.contains("void retranslateUi("));
	tmp << QString("    void retranslateUi(%1 *);").arg(widget_type) << endl;
	cpp << QString("    void Ui_%1::retranslateUi(%2 *%3)").arg(class_name).arg(widget_type).arg(class_name) << endl;
	do {
		if (src.atEnd()) { cout << "Error" << endl; return -1; }
		buffer = src.readLine();
		cpp << buffer << endl;
	} while (!buffer.contains("} // retranslateUi"));
	do {
		buffer = src.readLine();
		tmp << buffer << endl;
	} while (!src.atEnd());
	QFile h_file(file_info.absoluteFilePath());
	if (!h_file.open(QFile::WriteOnly | QFile::Text))
		{ cout << "Error writing file " << QByteArray().append(file_info.absoluteFilePath()).constData() << endl; return -1; }
	QTextStream h(&h_file);
	h << header << endl;
	cout << QByteArray().append(ui_info.fileName()).constData() << " > splitting successful" << endl;
	return 0;
}

int updateProjectFile(QString file_name, QString src_folder_name = QString())
{
	QFileInfo file_info(file_name);
	if (!file_info.exists())
		{ cout << "Invalid argument: " << QByteArray().append(file_name).constData() << endl; return -1; }
	QDir src_folder(QString("%1/%2").arg(file_info.absolutePath()).arg(src_folder_name));
	if (!src_folder.exists())
		{ cout << "Invalid argument: " << QByteArray().append(src_folder_name).constData() << endl; return -1; }
	QFile file(file_name);
	if (!file.open(QFile::ReadOnly | QFile::Text))
		{ cout << "Error opening file " << QByteArray().append(file_name).constData() << endl; return -1; }
	QFileInfoList ui_list = src_folder.entryInfoList(QStringList() << "*.ui", QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
	if (ui_list.isEmpty())
		{ cout << "No UI files found in folder " << QByteArray().append(src_folder_name).constData() << endl; return -1; }
	QTextStream ts(&file);
	QString pro = ts.readAll();
	file.remove();
	if (!file.open(QFile::WriteOnly | QFile::Text))
		{ cout << "Error writing file " << QByteArray().append(file_name).constData() << endl; return -1; }
	ts << pro;
	QString pro_ns = pro.remove(" ").remove("\t").remove("\n");
	
	for (int i = 0; i < ui_list.count(); ++i) {
		split(ui_list.at(i).absoluteFilePath());
	}
	if (!pro_ns.contains("exists($(QTDIR)/bin/splitui.exe){")) {
		ts << endl << "win32 {" << endl;
		ts << "\texists($(QTDIR)/bin/splitui.exe) {" << endl;
		ts << "\t\tQMAKE_UIC = splitui.exe" << endl;
		ts << "\t\tSOURCES += ";
		for (int i = 0; i < ui_list.count(); ++i) {
			if (!src_folder_name.isEmpty()) { ts << src_folder_name << "/"; }
			ts << "ui_" << QByteArray().append(ui_list.at(i).completeBaseName()).constData() << ".cpp";
			if (i < ui_list.count() - 1) { ts << " \\\n\t\t           "; }
		}
		ts << "\n\t}\n}";
		cout << QByteArray().append(file_info.fileName()).constData() << " > SplitUI win32 support added" << endl;
	} else {
		cout << QByteArray().append(file_info.fileName()).constData() << " > SplitUI win32 support found" << endl;
	}
	if (!pro_ns.contains("exists(/usr/bin/splitui){")) {
		ts << endl << "unix {" << endl;
		ts << "\texists(/usr/bin/splitui) {" << endl;
		ts << "\t\tQMAKE_UIC = splitui" << endl;
		ts << "\t\tSOURCES += ";
		for (int i = 0; i < ui_list.count(); ++i) {
			if (!src_folder_name.isEmpty()) { ts << src_folder_name << "/"; }
			ts << "ui_" << QByteArray().append(ui_list.at(i).completeBaseName()).constData() << ".cpp";
			if (i < ui_list.count() - 1) { ts << " \\\n\t\t           "; }
		}
		ts << "\n\t}\n}";
		cout << QByteArray().append(file_info.fileName()).constData() << " > SplitUI unix & macx support added" << endl;
	} else {
		cout << QByteArray().append(file_info.fileName()).constData() << " > SplitUI unix & macx support found" << endl;
	}
	return 0;
}

int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);
	
	if (app.arguments().count() <= 1) {
		cout << "Usage: splitui <class_name>.ui" << endl;
		cout << "       splitui -pro <project_name>.pro [-src <folder>]" << endl;
		return -1;
	} else {
		if (app.arguments().at(1) == "-pro")
			if (app.arguments().contains("-src")) {
				int i = 3;
				do {
					if (app.arguments().at(i) == "-src")
						return updateProjectFile(app.arguments().at(2), app.arguments().at(i + 1));
					i++;
				} while (i < app.arguments().count());
			} else return updateProjectFile(app.arguments().at(2));
		return split(app.arguments().at(1));
	}
	
	return 0;
}
