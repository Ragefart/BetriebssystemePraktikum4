
#include "pch.h"
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <chrono>
#include <time.h>
#include <future>
#include <semaphore.h>

using namespace std;

//prototypes
void reader(int position);
void writer();

shared_timed_mutex sharedmutex;
mutex m;	

//Array to read and write on
vector<int> readthis;
//Array to save current status of reader threads
vector<bool> activereaders;
//vector<thread> threads;			//not used
//vector<future<void>*> futures;	//not used
future<void> futures2[50];		//not satisfied with this
sem_t sem;
//Flag to see if s.th. is currently writing
bool writing = false;


int main()
{
	sem_init(sem);
	srand(time(NULL));
	//fills the readable vector
	int arraysize;
	cout << "Bitte die Groesse des Arrays eingeben" << endl;
	cin >> arraysize;

	for (int i = 0; i < arraysize; i++) {
		readthis.push_back(i);
	}

	int amountreaders, amountwriters = 0;
	cout << "Wie viele Reader sollen betrieben werden? " << endl;
	cin >> amountreaders;
	cout << "Wie viele Writer sollen betrieben werden? " << endl;
	cin >> amountwriters;
	cout << endl;
	const int amountthreads = amountreaders + amountwriters;
	//future<void>* futures2 = new future<void>[amountthreads];


	//starts reader threads
	if (amountreaders > 0) {
		for (int i = 0; i < amountreaders; i++) {
			activereaders.push_back(false);
			futures2[i] = async(launch::async, reader, i);
		}
	}
	//starts writer threads
	if (amountwriters > 0) {
		for (int i = 0; i < amountwriters; i++) {
			futures2[i + amountreaders] = async(launch::async, writer);
		}
	}

	cout << "Main abgeschlossen \n";
}


//Reads array in a backwards direction again and again
void reader(int position) {

	thread::id this_id = this_thread::get_id();
	cout << "Reader mit der ID " << this_id << " liest ab jetzt permanent. " << endl;
	int size = readthis.size();
	int o = 0;
	while (true) {
		//while a thread is not currently writing on the array

		m.lock();
		if (writing) {
			sem_wait(sem);
		}
		m.unlock();
		while (!writing) {
			activereaders[position] = true;
			cout << "Reader mit der ID " << this_id << " liest: " << readthis[(o%size)] << endl;
			o++;
			activereaders[position] = false;
			this_thread::sleep_for(chrono::milliseconds(30));
		}

	}
}


//Writes random numbers in the array from front to back
//sleeps different times each iteration
void writer() {
	int i = 1;
	bool breakthis = false;
	thread::id this_id = this_thread::get_id();
	cout << "Writer mit der ID " << this_id << " schreibt ab jetzt alle 1-5 Sekunden. " << endl;
	while (true) {
		breakthis = false;
		i++;
		this_thread::sleep_for(chrono::seconds((i % 5) + 1));
		//blocks when another thread is already writing
		if (!writing) {
			//only this thread is currently running
			sharedmutex.lock_shared();
			writing = true;
			//fills array with random values
			this_thread::sleep_for(chrono::milliseconds(150));
			//check if a reader is currently active and break the loop
			//in the case that one is
			for (bool active : activereaders) {
				if (active) {
					breakthis = true;
					break;
				}
			}
			if (breakthis) {
				break;
			}
			cout << "Writer mit der ID " << this_id << " beschreibt gerade das Array mit Zufallszahlen" << endl;
			for (int newnumber = 0; newnumber < readthis.size(); newnumber++) {
				readthis[newnumber] = rand() % 100;
			}
			for (int outnumber : readthis) {
				cout << outnumber << endl;
			}
			cout << endl << endl;
			this_thread::sleep_for(chrono::milliseconds(1500));
			//all other threads are running again
			sharedmutex.unlock_shared();
			writing = false;
			sem_post(sem);
		}

	}
}

