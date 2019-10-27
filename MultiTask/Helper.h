#pragma once

//  ���L�������X�e�[�^�X
#define	OK_SHMEM			0			// ���L������ ����/�j������
#define	ERR_SHMEM_CREATE	-1			// ���L������ Create�ُ�
#define	ERR_SHMEM_VIEW		-2			// ���L������ View�ُ�

#define	ON				1		// ON
#define	OFF				0		// OFF

#define Bitcheck(a,b)	(a >> b) & 1
#define Bitset(a,b)		a |= (1<<b)
#define Bitclear(a,b)		a &= ~(1<<b)


using namespace std;

class CHelper
{
public:
	CHelper();
	~CHelper();
	static void Str2Wstr(const std::string &src, std::wstring &dest);
	static void put_bmp_built(HDC hdc, HBITMAP hbmp, POINT dst_pt, POINT src_pt);
	static void put_bmp_stretch(HDC hdc, HBITMAP hbmp, POINT dst_pt, POINT src_pt, int retio_persent);
	static int	cmnCreateShmem(LPCTSTR, DWORD, HANDLE*, LPVOID*, DWORD*);	//���L�������ݒ�
	static int	cmnDeleteShMem(HANDLE*, LPVOID*);							//���L����������
};
