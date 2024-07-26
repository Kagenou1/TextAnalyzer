#include <iostream>
#include <fstream>
#include <filesystem>

#include <vector> 
#include <string>
#include <string_view>

#include <format>
#include <chrono>
#include <future>
#include <queue>
#include <tuple>

constexpr auto THREADS_DEFAULT = 4;
constexpr auto FILENAME_DEFAULT = "file.txt";

namespace fs = std::filesystem;
using namespace std;

class text_analyzer
{

	using position = pair<int, int>;

	struct compare
	{
		bool operator()(const int& a, const int& b)
		{
			return a > b;
		}
	};

	struct position_compare
	{
		bool operator()(const position& a, const position& b)
		{
			return a.first >= b.first && a.second >= b.second;
		}
	};

	using indexes_array = priority_queue<int, vector<int>, compare>;
	using positions_array = vector<position>;

public:

	text_analyzer() {};

	text_analyzer(const string_view _file_path) : text_analyzer()
	{
		file_path = _file_path;
	}

	indexes_array get_indexes() const
	{
		return substr_indexes;
	}

	static positions_array get_positions(indexes_array indexes)
	{
		positions_array substr_positions{};
		
		return substr_positions;
	}

	int find_substr(const string substr, const size_t threads_count = 1)
	{
		if (!filesystem::exists(file_path) || filesystem::file_size(file_path) == 0)
			return -1;

		if (substr.size() == 0)
			return 0;

		filesystem::path filesystem_path(file_path);

		auto text_size = filesystem::file_size(file_path);
		cout << format("Text size - {} bytes\n", text_size);

		vector < future<int>> threads(threads_count);

		size_t rem = text_size % threads_count;
		size_t string_size = text_size / threads_count;
		size_t pos = 0;

		atomic<int> count = 0;

		for (auto& my_thread : threads)
		{
			my_thread = async(launch::async, &text_analyzer::_find_substr, this, substr, pos, string_size + (rem > 0 ? 1 : 0));
			pos += string_size + (rem > 0 ? 1 : 0);
			if (rem > 0)
				rem--;
		}
		for (auto& my_thread : threads)
		{
			my_thread.wait();
			count.fetch_add(my_thread.get());
		}

		return count;
	}

private:

	int _find_substr(const string_view substr, const size_t offset = 0, const size_t bytes_to_read = 0)
	{
		//auto begin = chrono::system_clock::now();

		int thread_id = thread_count.load();
		thread_count.fetch_add(1);

		//cout << format("Thread {} from pos {} - {} bytes\n", thread_id, offset, bytes_to_read);

		ifstream file(file_path.native(), ios::binary);
		file.seekg(offset, ios_base::beg);

		size_t substr_size = substr.size();

		if (file.eof() || substr_size == 0)
			return 0;

		int symbols_found = 0;
		atomic<int> count = 0;

		//cout << format("First symbol thread {} - {}\n", thread_id, (char)file.peek());

		for (size_t i = 0; ((bytes_to_read > 0 ? i < bytes_to_read : true) || symbols_found != 0) && !file.eof(); i++)
		{
			if (symbols_found == substr_size)
			{

				while (flag_write.test_and_set(memory_order_acquire)) {}
				substr_indexes.push(int(offset + i - substr_size));
				
				flag_write.clear(memory_order_release);				

				count.fetch_add(1);
				symbols_found = 0;
			}
			char ch = file.get();
			if (ch == substr[symbols_found])
				symbols_found++;
			else
				symbols_found = 0;
		}

		//auto end = chrono::system_clock::now();

		//cout << format("Symbol after last thread {} - {}", thread_id, (char)file.peek()) << endl;

		file.close();

		//cout << format("Thread {} time - {} mcs\n", thread_id, chrono::duration_cast<chrono::microseconds>(end - begin).count());

		return count;
	}

	vector<int> get_line_feeds()
	{
		vector<int> line_feeds_array{};
		ifstream file(file_path.native(), ios::binary);
		int count = 0;
		while (!file.eof())
		{
			if (file.get() == '\n')
				line_feeds_array.push_back(count);

			count++;
		}


	}


	indexes_array substr_indexes{};
	positions_array* substr_positions = nullptr;
	inline static atomic<int> thread_count = 1;

	fs::path file_path;

	atomic_flag flag_write = ATOMIC_FLAG_INIT;
};


int main(int argc, char** argv)
{
	fs::path file_path{};
	if (argc >= 2)
		file_path = argv[1];
	else
	{
		string_view path_to_exe = argv[0];
		auto path_to_folder = filesystem::path(path_to_exe).parent_path();
		for (const auto& p : fs::recursive_directory_iterator(path_to_folder))
			if (p.path().filename() == FILENAME_DEFAULT)
			{
				file_path = p.path();
				break;
			}
	}
	if (file_path == " " || !filesystem::exists(file_path))
	{
		cout << "Invalid file path" << endl;
		return -1;
	}
	cout << format("Path to .txt file - {}", file_path.string()) << endl;
	string substr{};
	if (argc >= 3)
	{
		substr = string(argv[2]);
		erase(substr, ' ');
	}
	else
		substr = "as";

	int threads_count{};

	if (argc >= 4)
		threads_count = stoi(argv[3]);

	if(threads_count == 0)
		threads_count = THREADS_DEFAULT;


	text_analyzer text1(file_path.string());

	cout << format("Threads count - {}", threads_count) << endl;
	cout << format("String to find - {}", substr) << endl;

	auto begin = chrono::system_clock::now();

	cout << format("Indexes found - {}", text1.find_substr(substr, threads_count)) << endl;

	auto end = chrono::system_clock::now();

	cout << format("Program time - {} mcs\n", chrono::duration_cast<chrono::microseconds>(end - begin).count());

	auto text_indexes = text1.get_indexes();

	/*while(!text_indexes.empty())
	{
		cout << text_indexes.top() << '\t';
		text_indexes.pop();
	}
	cout << endl;*/

	return 0;
}