#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

#include "../TinyTL/TObject.hpp"
#include "../TinyTL/MyLib.hpp"

//���ں���SetConsoleOutputCP(65001);����cmd����Ϊutf8

using namespace Gdiplus;
using namespace std;

#pragma comment(lib,"User32.lib")
#pragma comment(lib,"ComCtl32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "wsock32.lib")

#define IDT_TIMER1 12

// �ַ�������
vector<string> Val;

//�洢@var���ֵı�������ֵVal���±ꡣ
map<string, string> var;

int cxScreen,cyScreen;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//�ж�s�Ƿ���Element�������ã��ú���Ϊ���ú��������ڷ���ĳ���ڵ㡣
bool Is_Fun_Element(string s) {
  return true;
}

//���һ�����ӵ�Զ�̷�����
string lasthost = "";

string Get_Host(string s) {
  string tmp;
  for (int i=0; i<s.length(); i++) {
    if (s[i]=='@') {
      tmp=s.substr(i+1);
      if (tmp == "_VAL") {
       i = atoi(s.substr(i+4).c_str());
        lasthost = Val[i];
        return lasthost;
      } else {
        lasthost = var[tmp];
        return lasthost;
      }
    }
  }
  return lasthost;
}

//ȡ�ú�����
string Get_Fun_Call(string s) {
  for (int i = 0; i < s.length(); i++) {
    if (s[i] == '@') {
      return s.substr(0, i);
    }
  }
  return s;
}

//���s�Ƿ����ch
bool HasChar(string s, char ch) {
  for (int i=0; i<=s.length(); i++) {
    if (s[i] == ch)
      return true;
  }
  return false;
}

class Connection {
  public:
    string Host;
    SOCKET sock;
    Connection(string s);
    ~Connection();
    //���÷�����Զ�̺������õ�����ֵ
    string RPC(string s);
};

vector<class Connection*> vc;

//���캯��
Connection::Connection(string h) {
  Host = h;
  
  //��ʼ��DLL
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
  //�����׽���
  SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  //���������������
  sockaddr_in sockAddr;
  memset(&sockAddr, 0, sizeof(sockAddr));  //ÿ���ֽڶ���0���
  sockAddr.sin_family = PF_INET;
  sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  sockAddr.sin_port = htons(9999);
  connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
}

Connection::~Connection() {
  //�ر��׽���
  closesocket(sock);
  //��ֹʹ�� DLL
  WSACleanup();
}

string Connection::RPC(string s) {
  //���շ��������ص�����
  char szBuffer[64] = {0};
  send(sock, s.c_str(), s.length(), 0);
  recv(sock, szBuffer, 64, NULL);
  //������յ�������
  printf("Message form server: %s\n", szBuffer);
  return szBuffer;
}


// ����S��ע�⣬��������ȼ���
string Evaluate(string f) {	  
  for (int i=0; i<f.length(); i++) {
    //��ֵ���
    if (f[i]=='=') {
      string v = f.substr(0, i);
      
      //�б������ֵ�Ԫ�ظ�ֵ
      if (HasChar(v, '|') == true) {
        break;
      }
      
      //��$��ͷ����ʾΪ�ڵ����Ը�ֵ
      if (v[0]=='$') {
        break;
      }
      
      for (auto j: var) {
        if (j.first == v) {
          //������ֵ
          var[v] = Evaluate(f.substr(i+1, f.length()));
          break;
        }
      }
      
      break;
    }
  }
  
  //�ڵ�Insert Delete 
  if (f[0]=='$') {
  
  }
  
  //�õ�Զ�̷�������
  string host = Get_Host(f);
  
  //�õ��������Ͳ���
  string n = Get_Fun_Call(f);
  
  for (auto i:vc) {
    if (i->Host==host) {
      return i->RPC(n);
    } else {
      class Connection *con = new Connection(host);
      vc.push_back(con);
      return con->RPC(n);
    }
  }
  
  return "";
}

class Proc {
public:
  string name;    //��������������"@init" "" ...
  vector<string> s; //�ѹ��̵������Կո����֣����vector<string> s
  void Call();
};

void Proc::Call(){
	cout << "calling .." << name << "\n";
	for (auto i: s) {
		Evaluate(i);
	}
}

//����������-1��ʾû�ҵ���
int InVal(string s) {
	for (int i=0; i<Val.size(); i++) {
		if (Val[i] == s) {
			return i;
		}
	}
	return -1;
}

//�����б�
vector<class Proc *> vp;

/*���ַ����洢��vector<string> val�У����ö�Ӧ��vector�±����滻�ַ�����
  ��ô����Ŀ����Ϊ�˷������ʷ��������Կո�Ϊ�����*/
string String_2_Int(string line){
  int i = 0;
  string str = "";
  short n=0;
   
  while (1) {
    string s = "";
    
    while (line[i] != '\"') {
      str+=line[i];
      if (i == (line.length()-1))
        return str;
      i++;
    }
    
    //�˴�����⵽��һ����
    i++;
    
    while (line[i] != '\"') {
      s+=line[i];
      if (i == (line.length()-1)) {
        cout << " �����ɶԡ�\n";
        exit(0);
      }
      i++;
    }
    
    //�˴����ڶ���"����⵽
    int m = InVal(s);
    
    //����������   
    char buf[10];
    
    if (m >= 0 ) {
      sprintf(buf,"%d",m);
      str=str+"_VAL"+buf;				
    } else {
      Val.push_back(s);
      n = Val.size();		
      sprintf(buf,"%d",n-1);
      str=str+"_VAL"+buf;		
    }
    
    if (i == (line.length()-1))
      break;
    i++;
  }
  return str;
}

class GUI_Element {
public:  
  
  unsigned short Level;         //�ؼ��ڵڼ�Level��
  string Name;                  //�ؼ����� �����ܵ�ValΪ"<window>" "<text>" "<div>" ...
  map<string, string> Property; //�ؼ�����  titile=1, 1ΪVal��index.
  
  //�ؼ���������  ��λ�Ƿֱ���  left top right bottom
  int l;
  int t;
  int r;
  int b;
  
  class GUI_Element *parent;    //���ڵ�
  class GUI_Element *child;     //��һ���ӽڵ�
  class GUI_Element *brother;   //��һ���ֵܽڵ�

  GUI_Element(string line);  
  
};

//line���ļ��е�һ�У�1 window title="��һ�� ����"
GUI_Element::GUI_Element(string line) {  
  string s = Get_First(line);
                            
  for (int i=0; i<s.length(); i++){
    if (!isdigit(s[i])) {
      cout << "You should start with Level number.\n";
      return;
    }
  }
  
  Level = atoi(s.c_str());
  
  string tmp = Skip_Blank(line.substr(s.length()));
  Name = Get_First(tmp);
  
  tmp = Skip_Blank(tmp.substr(Name.length()));
  string f = Get_First(tmp);
  //�������һ������
  while (f != tmp) {
    for (int i=0; i<f.length(); i++) {
      if (f[i]=='=') {
        //cout << f.substr(i+1, f.length()) << "\n";
        Property[f.substr(0, i)] = f.substr(i+1, f.length());
        break;
      }
    }
    tmp = Skip_Blank(tmp.substr(f.length()));
    f = Get_First(tmp);
  }
  for (int i=0; i<f.length(); i++) {
    if (f[i]=='=') {
      Property[f.substr(0, i)] = f.substr(i+1, f.length());
    }
  }
  cout << Level << ", "<< Name << "\n";
  for(auto iter = Property.begin(); iter != Property.end(); iter++) {
    cout << iter->first << " : " << iter->second << endl;
  }
}

struct Window_Element {
  HWND  hwnd;
  class GUI_Element *head;
};

//�����б�
vector<struct Window_Element *> v;

//����Window
void Draw_Window(struct Window_Element *w_e) {
  
  class GUI_Element *tmp = w_e->head;
  
  //���ƶ��㴰��
  if (tmp->Name == "WINDOW") {
    MSG         msg;
    string      title = "Test Window";
    
    int x = atof(tmp->Property["left"].c_str()) * cxScreen;
    int y = atof(tmp->Property["top"].c_str()) * cyScreen;
    int w = (atof(tmp->Property["right"].c_str()) - atof(tmp->Property["left"].c_str())) * cxScreen;
    int h = (atof(tmp->Property["bottom"].c_str()) - atof(tmp->Property["top"].c_str())) * cyScreen;
    
    w_e->hwnd = CreateWindow(
      TEXT("MyClass"),   			  // window class name
      TEXT(title.c_str()),  		        
      WS_OVERLAPPEDWINDOW,     	// window style
      x,           	// initial x position
      y,           	// initial y position
      w,           	// initial x size
      h,           	// initial y size
      NULL,						            // parent window handle
      NULL,                     	// window menu handle
      NULL,          			  	    // program instance handle
      NULL);                    	// creation parameters			
      
    /*
    SetTimer(w.hwnd,             // handle to main window 
      IDT_TIMER1,            // timer identifier 
      1000,                 // 10-second interval 
      (TIMERPROC) NULL);     // no timer callback 

    SetWindowText(w.hwnd, "aaa");    */
  
    ShowWindow(w_e->hwnd, 1);
    UpdateWindow(w_e->hwnd);
		
		//����һ��WM_PAINTER��Ϣ�������ӿؼ���
    //InvalidateRect(w_e->hwnd, NULL, TRUE);

    //��Ϣѭ��
    while(GetMessage(&msg, NULL, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
}

//����Window������ӿؼ���ͬһ�㣬����ƵĿ��ܻḲ���Ȼ��Ƶ�;  �ȸ����ӣ����ֺ�ܡ�
void Draw_Element(class GUI_Element *tmp, HDC hdc, HWND hwnd) {

  //ʹ��gdi�������ƾ���
  if (tmp->Name == "RECT") {   
    //����ֵ
    int pt = tmp->parent->t;
    int pl = tmp->parent->l;
    int pr = tmp->parent->r;
    int pb = tmp->parent->b;
    
    int l = atof(tmp->Property["left"].c_str()) * (pr - pl) + pl;
    int t = atof(tmp->Property["top"].c_str()) * (pb - pt) + pt;
    int r = atof(tmp->Property["right"].c_str()) * (pr - pl) + pl;
    int b = atof(tmp->Property["bottom"].c_str()) * (pb - pt) + pt;
    
		tmp->l = l;
		tmp->t = t;
		tmp->r = r;
		tmp->b = b;
  }
  
  //RECT��RECTANGEL��������ǰ��ֻ����λʹ�ã��������ƣ����߻���ƾ��Ρ�
 
  if (tmp->Name == "RECTANGLE") {
    //����ֵ
    int pt = tmp->parent->t;
    int pl = tmp->parent->l;
    int pr = tmp->parent->r;
    int pb = tmp->parent->b;
    
    int l = atof(tmp->Property["left"].c_str()) * (pr - pl) + pl;
    int t = atof(tmp->Property["top"].c_str()) * (pb - pt) + pt;
    int r = atof(tmp->Property["right"].c_str()) * (pr - pl) + pl;
    int b = atof(tmp->Property["bottom"].c_str()) * (pb - pt) + pt;

		cout << tmp->parent->Name << " parent name .. \n";
    cout << pl << " "<< pt << " " << pr  << " " << pb <<"  parent ... RECTANGLE\n";
    cout << l << " "<< t  << " " << r  << " " << b <<" local ... RECTANGLE\n";

		tmp->l = l;
		tmp->t = t;
		tmp->r = r;
		tmp->b = b;
    
    Rectangle(hdc,l,t,r,b);
    //Draw_Rectangle();
  }
  
  if (tmp->Name == "TEXT") {
    string s;
    
    s = tmp->Property["caption"];

    for (auto it: var) {
      if (it.first == s) {
        s = it.second;
      }
    }
   
    tmp->t = tmp->parent->t;
    tmp->l = tmp->parent->l;
    tmp->r = tmp->parent->r;
    tmp->b = tmp->parent->b;
    
    RECT re;  
    re.left=tmp->l; re.top=tmp->t; re.right=tmp->r; re.bottom=tmp->b;
    
    int i;
    if (s.substr(0, 4) == "_VAL")
       i = atoi(s.substr(4).c_str());
    //TextOut(hdc, l, t, s.c_str(), s.length());
    DrawText(hdc, Val[i].c_str(), Val[i].length(), &re, DT_LEFT);
  }
  
  if (tmp->Name == "LINE") {
  }
  
  //����
  if (tmp->Name == "A") {
    cout<< "A...\n";
  }
  
	if (tmp->Property["sleep"]!="")
		Sleep(atoi(tmp->Property["sleep"].c_str()));
	
  if (tmp->child)
    Draw_Element(tmp->child, hdc, hwnd);
  if (tmp->brother)
    Draw_Element(tmp->brother, hdc, hwnd);
}

//�ҵ�hwnd��Ӧ��Window_Element�������±꣬�������-1��ʾû���ҵ���
int Find_Window(HWND hwnd) {
  for (int i=0; i< v.size(); i++){
    if (v[i]->hwnd == hwnd) {
      return i;
    }
  }
  return -1;
}

//��ȡGUI�����ļ����������νṹ��
void read_gui(char *gui){
  ifstream in(gui);
  string line;
      
  MyAssert(Get_First("hedllo world"), "hedllo");
  
  if (in) {
      
    class GUI_Element *last = NULL;
    // line�в�����ÿ�еĻ��з�
    while (getline (in, line)) { 
    
      line = Skip_Blank(line);
      cout << line << endl;
          
      if (line == "")
        continue;
      // ����ͷ��ʾע��
      if (line[0] == ';')
        continue;                
        
      //��������
      if (line=="@var") {
        while (getline (in, line)) {
          line = Skip_Blank(line);
          cout << line << "\n";          
          
          if (line == "")
            continue;
          if (line[0] == ';')
            continue;
          if (line == "end")
            break;
          
          string l = String_2_Int(line);
          cout << l << " ..\n";
          
          string f = Get_First(l);
          //�������һ������
          while (f != l) {
            Evaluate(f);
            l = Skip_Blank(l.substr(f.length()));
            f = Get_First(l);
          }
          Evaluate(f);
        }
      }

      //ͼ������
      if (line=="@gui") {        
        while (getline (in, line)) {
          line = Skip_Blank(line);
          cout << line << "\n";
					
          if (line == "")
            continue;
          if (line[0] == ';')
            continue;   
          if (line=="end")
            break;          
          
          string l = String_2_Int(line);
          cout << l << "\n";

          class GUI_Element *con = new GUI_Element(l);
          
          //����ؼ�
          if (con->Level == 1 && con->Name=="WINDOW") {
            con->parent=NULL;
            con->child=NULL;
            con->brother=NULL;
            last = con;
            struct Window_Element *e = (struct Window_Element *)malloc(sizeof(struct Window_Element));
            e->hwnd = 0;
            e->head = con;
            v.push_back(e);
            continue;
          } 
          
          //�Ƕ���ؼ�
          if (last != NULL) {
            //last���ӿؼ�
            if (con->Level - last->Level == 1) {
              con->parent=last;
              con->child=NULL;
              con->brother=NULL;
              last->child = con;
              last = con;
              continue;
            }

            //last���ֵܿؼ�
            if (con->Level == last->Level) {
              last->brother=con;
              
              con->parent=last->parent;
              con->child=NULL;
              con->brother=NULL;
              last = con;
              continue;
            }
            //last�ϲ�ؼ�
            if (con->Level < last->Level && con->Level > 1) {
              while (con->Level != last->Level)
                last = last->parent;
              last->brother = con;
              con->parent=last->parent;
              con->child=NULL;
              con->brother=NULL;
              last = con;
              continue;
            }
            //�ؼ���δ���
            cout << con->Level  << "..\n";
            cout << last->Level  << "..\n";
            if (con->Level - last->Level > 1) {
              cout << "Level error.\n";
              return;
            }
          }
        }
      }
      
      //��ʼ������
      if (line=="@init") {
        while (getline (in, line)) {
          line = Skip_Blank(line);
          cout << line << "\n";
          if (line == "")
            continue;
          if (line[0] == ';')
            continue;
          if (line=="end")
            break;
          string l = String_2_Int(line);
          continue;
        }
      }

      //��@��ͷ������Ϊ@init--��ʼ���������߼�����괦��������
      if (line[0] == '@') {
        class Proc *p = new(class Proc);
        p->name = line.substr(1);
        
        while (getline (in, line)) {
          line = Skip_Blank(line);
          cout << line << "\n";
          if (line == "")
            continue;
          if (line[0] == ';')
            continue;
          if (line=="end")
            break;
          
          string l = String_2_Int(line);

          string f = Get_First(l);
          //�������һ������
          while (f != l) {
            p->s.push_back(f);
            l = Skip_Blank(l.substr(f.length()));
            f = Get_First(l);
          }
        }
        vp.push_back(p);
      }
    }
  } else {
      cout <<"no such file" << endl;
  }
}

int main(int argc, char **argv) {
  SetConsoleOutputCP(65001);
  cxScreen=GetSystemMetrics(SM_CXSCREEN);
  cyScreen=GetSystemMetrics(SM_CYSCREEN);
  if (argc == 1) {
    cout << "Need file name as args.\n";
    exit(1);
  }
  read_gui(argv[1]);
  
  WNDCLASS            wndClass;
  
  wndClass.style          = CS_HREDRAW | CS_VREDRAW;
  //ͬһ�����๫��һ�����ڴ�������
  wndClass.lpfnWndProc    = WndProc;
  wndClass.cbClsExtra     = 0;
  wndClass.cbWndExtra     = 0;
  wndClass.hInstance      = NULL;
  wndClass.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
  wndClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
  wndClass.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wndClass.lpszMenuName   = NULL;
  wndClass.lpszClassName  = TEXT("MyClass");
  
  RegisterClass(&wndClass);
  Draw_Window(v[0]);

  //�ͷ��ڴ�
  
  return 0;
}

//����p�����Ŀؼ�
GUI_Element * Find_Element(POINT p, GUI_Element *e) {
  GUI_Element *last = NULL;

  cout << "e->name = " << e->Name << "\n";
  
  if ((p.x >= e->l) && (p.y >= e->t) && (p.x <= e->r) && (p.y <= e->b))
      last = e;
    
  if (e->child != NULL) {
    if (e->child->Name == "RECTANGLE") {
      GUI_Element *tmp = Find_Element(p, e->child);
      if (tmp!=NULL)
        last = tmp;
    }
  } 
  
  if (e->brother!= NULL) {
    if (e->brother->Name == "RECTANGLE") {
      GUI_Element *tmp = Find_Element(p, e->brother);
      if (tmp!=NULL)
        last = tmp;
    }
  } 
  
  return last;
}

// vp�������Ϊname�Ĺ���
class Proc * Find_Proc(string name) {
		for (auto i:vp) {
			if (i->name == name)
				return i;
		}
		return NULL;
}

//ͬһ�������๫��һ�����ڴ�������
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, 
   WPARAM wParam, LPARAM lParam)
{
  int i;
  string s;
  RECT r;  
  
  switch(message) {
    //�ƶ���������
    case WM_SYSCOMMAND:
      i = Find_Window(hWnd);
      
      //ȡ�ô��ڿͻ�������
      GetClientRect(hWnd, &r);
      
      //ȡ���ƶ����޸Ĵ���Ŀͻ������ꡣ
      v[i]->head->l = r.left;
      v[i]->head->t = r.top;
      v[i]->head->t = r.right;
      v[i]->head->b = r.bottom;
			
      //InvalidateRect(hWnd, NULL, TRUE);
      
      return DefWindowProc(hWnd, message, wParam, lParam);
      
    case WM_LBUTTONDOWN:
      
      POINT point;
      
      GetCursorPos(&point);            // ��ȡ���ָ��λ�ã���Ļ���꣩
      ScreenToClient(hWnd, &point);    // �����ָ��λ��ת��Ϊ��������
      cout << point.x << " "<< point.y << "\n";
      
      i = Find_Window(hWnd);
      //����Window_Element���õ�����������ڵĿؼ����������䴦��������
      //������ʽ���ȸ����ӣ����ֺ�ܡ�
      class GUI_Element * tmp;
      tmp = Find_Element(point, v[i]->head);
      
      s = tmp->Property["click"];
			cout << s  << " s = \n";
      int j;
      if (s.substr(0, 4) == "_VAL") {
        j = atoi(s.substr(4).c_str());
        cout<< j << " j =\n";
      }
      cout << Val[j] << " is clicked\n";
      
			class Proc * p;
			p = Find_Proc(Val[j]);
      if (p!=NULL)
				p->Call();
			
			return 0;
    case WM_LBUTTONDBLCLK:
      return 0;
    case WM_RBUTTONDOWN:
      return 0;
    case WM_RBUTTONDBLCLK:
      return 0;
    case WM_MBUTTONDOWN:
      return 0;
    case WM_MBUTTONDBLCLK:
      return 0;
    //����Edit�ؼ����ı�
    case WM_SETTEXT:
      return 0;
    case WM_TIMER: 
      //InvalidateRect(hWnd, NULL, TRUE);
      return 0;
      
    // WINDOW����ͼ��Ԫ�صĻ���
    case WM_PAINT:
      PAINTSTRUCT pt;
      HDC hdc;
      
      hdc=BeginPaint(hWnd,&pt);
      SetTextColor(hdc,RGB(255,0,0));
      SetBkColor(hdc,RGB(0,255,0));
      SetBkMode(hdc,TRANSPARENT);
      
      i = Find_Window(hWnd);
            
      //ȡ�ô��ڿͻ�������
      GetClientRect(hWnd, &r);
            
      //�洢��ǰ���ڿͻ�����top left right bottom
      v[i]->head->l = r.left;
      v[i]->head->t = r.top;
      v[i]->head->r = r.right;
      v[i]->head->b = r.bottom;

			Sleep(atoi(v[i]->head->Property["sleep"].c_str()));
      Draw_Element(v[i]->head->child, hdc, hWnd);
      
      EndPaint(hWnd,&pt);
			return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}