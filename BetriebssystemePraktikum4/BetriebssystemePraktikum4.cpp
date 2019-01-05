
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

using namespace std;

//prototypes
void reader();
void writer();

shared_timed_mutex sharedmutex;
//mutex m;							not used

vector<int> readthis;
//vector<thread> threads;			//not used
//vector<future<void>*> futures;	//not used
future<void> futures2[50];		//not satisfied with this

//Flag to see if s.th. is currently writing
bool writing = false;

int main()
{
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
			//futures.push_back(new future<void>());
			//*futures[i] = async(launch::async, reader);
			//future<void> readers = async(launch::async, reader);
			//thread readers(reader);
			//threads.push_back(readers);
			futures2[i] = async(launch::async, reader);
		}
	}
	//starts writer threads
	if (amountwriters > 0) {
		for (int i = 0; i < amountwriters; i++) {

			//future<void> writers = async(launch::async, writer);
			//thread writers(writer);
			//threads.push_back(writers);
			futures2[i + amountreaders] = async(launch::async, writer);
		}
	}
	//joins all threads
	/*for (thread& t : threads) {
		t.join();
	}*/
	cout << "Main abgeschlossen \n";
}


//Reads array in a backwards direction again and again
void reader() {

	thread::id this_id = this_thread::get_id();
	cout << "Reader mit der ID " << this_id << " liest ab jetzt permanent. " << endl;
	int size = readthis.size();
	int o = 0;
	while (true) {
		//while a thread is not currently writing on the array
		while (!writing) {
			cout << "Reader mit der ID " << this_id << " liest: " << readthis[(o%size)] << endl;
			o++;
			this_thread::sleep_for(chrono::milliseconds(30));
		}
	}
}


//Writes random numbers in the array from front to back
//sleeps different times each iteration
void writer() {
	int i = 1;
	thread::id this_id = this_thread::get_id();
	cout << "Writer mit der ID " << this_id << " schreibt ab jetzt alle 1-5 Sekunden. " << endl;
	while (true) {
		i++;
		this_thread::sleep_for(chrono::seconds((i % 5) + 1));
		//blocks when another thread is already writing
		if (!writing) {
			//only this thread is currently running
			//m.lock();
			sharedmutex.lock_shared();
			writing = true;
			//fills array with random values
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
			//m.unlock();
			sharedmutex.unlock_shared();
			writing = false;
		}

	}
}

