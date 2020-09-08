#include "Gems.h"
#include <QtWidgets/QApplication>
#include <QDesktopWidget>
#include <windows.h>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  srand(time(NULL));
  Gems gems;
  gems.show();

  QRect screenGeometry = QApplication::desktop()->screenGeometry();
  int x = (screenGeometry.width() - gems.width()) / 2;
  int y = (screenGeometry.height() - gems.height()) / 2;
  gems.move(x, y);
 
  #ifndef _DEBUG
    HANDLE hLogFile;
    hLogFile = CreateFile(L"Memory Leaks.txt", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, hLogFile);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, hLogFile);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, hLogFile);
  #endif
  return a.exec();
}
