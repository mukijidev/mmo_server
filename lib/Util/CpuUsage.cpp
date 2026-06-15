#include "CpuUsage.h"


// 생성자 확인대상 ; 프로세스 핸들, 미입력시 자기 자신
CpuUsage::CpuUsage(HANDLE hProcess)
{
    if (hProcess == INVALID_HANDLE_VALUE)
    {
        // 프로세스 핸들 입력이 없다면 자기 자신을ㄷ ㅐ상으ㅗㄹ
        _hProcess = GetCurrentProcess();
    }

    //프로세서 개수를 확인한다.
    //프로세스 실행률 계산시 cpu 개수로 나누기를 하여 실제 사용률을 구함
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    _iNumberOfProcessors = SystemInfo.dwNumberOfProcessors;
    _fProcessorTotal = 0;
    _fProcessorUser = 0;
    _fProcessorKernel = 0;
    _fProcessTotal = 0;
    _fProcessUser = 0;
    _fProcessKernel = 0;
    _ftProcessor_LastKernel.QuadPart = 0;
    _ftProcessor_LastUser.QuadPart = 0;
    _ftProcessor_LastIdle.QuadPart = 0;
    _ftProcess_LastUser.QuadPart = 0;
    _ftProcess_LastKernel.QuadPart = 0;
    _ftProcess_LastTime.QuadPart = 0;

    UpdateCpuTime();
}

void CpuUsage::UpdateCpuTime()
{
    // 프로세서 사용률을 갱신한다
    // 본래의 사용 구조체는 FILETIME 이지만 ULARGE_INTEGER와 구조가 같으므로 이를 사용
    // FILETIME 구조체는 100 나노세컨드 단위의 시간 단위를 표현하는 구조체임
    ULARGE_INTEGER Idle;
    ULARGE_INTEGER Kernel;
    ULARGE_INTEGER User;

    // 아이들 커널 유저 시스템 사용시간을 구한다
    if (GetSystemTimes((PFILETIME)&Idle, (PFILETIME)&Kernel, (PFILETIME)&User) == false)
    {
        return;
    }

    // 커널타임에는 아이들 타임이 포함됨 // idle thread도 커널에서 도는 쓰레드니까
    ULONGLONG KernelDiff = Kernel.QuadPart - _ftProcessor_LastKernel.QuadPart;
    ULONGLONG UserDiff = User.QuadPart - _ftProcessor_LastUser.QuadPart;
    ULONGLONG IdleDiff = Idle.QuadPart - _ftProcessor_LastIdle.QuadPart;

    ULONGLONG Total = KernelDiff + UserDiff;
    ULONGLONG TimeDiff;
    if (Total == 0)
    {
        _fProcessorUser = 0.0f;
        _fProcessorKernel = 0.0f;
        _fProcessorTotal = 0.0f;
    }
    else
    {
        // 커널 타임에 아이들 타임이 있으므로 빼서 계산.
        _fProcessorTotal = (float)((double)(Total - IdleDiff) / Total * 100.0f);
        _fProcessorUser = (float)((double)UserDiff / Total * 100.0f);
        _fProcessorKernel = (float)((double)(KernelDiff - IdleDiff) / Total * 100.0f);
    }
    // float temp value로 표현하신건가
    _ftProcessor_LastKernel = Kernel;
    _ftProcessor_LastUser = User;
    _ftProcessor_LastIdle = Idle;

    //----------------------------------------------
    // 지정된 프로세스 사용률을 갱신한다.
    //----------------------------------------------
    ULARGE_INTEGER None;
    ULARGE_INTEGER NowTime;

    GetSystemTimeAsFileTime((LPFILETIME)&NowTime);
    GetProcessTimes(_hProcess, (LPFILETIME)&None, (LPFILETIME)&None, (LPFILETIME)&Kernel, (LPFILETIME)&User);
    TimeDiff = NowTime.QuadPart - _ftProcess_LastTime.QuadPart;
    UserDiff = User.QuadPart - _ftProcess_LastUser.QuadPart;
    KernelDiff = Kernel.QuadPart - _ftProcess_LastKernel.QuadPart;
    Total = KernelDiff + UserDiff;
    _fProcessTotal = (float)(Total / (double)_iNumberOfProcessors / (double)TimeDiff * 100.0f);
    _fProcessKernel = (float)(KernelDiff / (double)_iNumberOfProcessors / (double)TimeDiff * 100.0f);
    _fProcessUser = (float)(UserDiff / (double)_iNumberOfProcessors / (double)TimeDiff * 100.0f);

    _ftProcess_LastTime = NowTime;
    _ftProcess_LastKernel = Kernel;
    _ftProcess_LastUser = User;
}

