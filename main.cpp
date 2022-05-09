#include <iostream>
#include <vector>
#include <windows.h>

using namespace std;

CRITICAL_SECTION cs;
HANDLE* markedEvents;
HANDLE* closeThreadEvents;
HANDLE continueEvent;
static vector<int> array;

struct numberToThread{
    numberToThread(int number_) {
        number = number_;
    }
    int number;
};

DWORD WINAPI marker(void* params_){
    numberToThread params = *reinterpret_cast<numberToThread*>(params_);
    int number = params.number;
    int marked = 0;
    srand(number);

    EnterCriticalSection(&cs);
    while(true) {
        int random = rand();
        random %= array.size();
        if (array[random] == 0) {
            Sleep(5);
            array[random] = number;
            Sleep(5);
            marked++;
        } else {
            cout << "Poryadkoviy nomer: " << number <<std::endl;
            cout << "Kolichestvo pomechennih elementov: " << marked <<std::endl;
            cout << "Index elementa kotoriy nevozmojno pometit: " << random + 1 <<std::endl;
            LeaveCriticalSection(&cs);

            SetEvent(markedEvents[number - 1]);

            HANDLE* possibleOptions = new HANDLE[2];
            possibleOptions[0] = continueEvent;
            possibleOptions[1] = closeThreadEvents[number - 1];
            DWORD option = WaitForMultipleObjects(2, possibleOptions, FALSE, INFINITE);
            if (option == WAIT_OBJECT_0 + 1){
                break;
            }
        }
    }
    for (int i = 0; i < array.size(); i++){
        if (array[i] == number)
            array[i] = 0;
    }

    return 0;
}

vector<HANDLE> start_threads(int count){
    vector<HANDLE> threads_handles(count);
    for (int i = 0; i < count; i++){
        HANDLE hThread;
        DWORD IDThread;
        hThread = CreateThread(
                NULL,
                0,
                marker,
                (void*)new numberToThread(i + 1),
                0,
                &IDThread);
        if(hThread != NULL) {
            cout << "Thread " << i + 1 << " created successfully" << endl;
            threads_handles[i] = hThread;
        }
        else {
            cout << "Something went wrong. Error code: " << GetLastError();
        }
    }
    return threads_handles;
}

HANDLE* CreateEvents(int count, bool manualReset, bool initialState){
    HANDLE* events = new HANDLE[count];
    for (int i = 0; i < count; i++){
        events[i] = CreateEventA(NULL, manualReset, initialState, NULL);
    }
    return events;
}

void showArray(vector<int>& v){
    for (int i = 0; i < v.size(); i++)
        cout << v[i] << " ";
    cout << endl;
}

void SetRemovedEvents(vector<HANDLE>& removedEvents){
    for (int i = 0; i < removedEvents.size(); i++){
        SetEvent(removedEvents[i]);
    }
}

