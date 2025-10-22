/**
    @mainpage 
    @author Samii Shabuse <sus24@drexel.edu>
    @date October 3, 2025 

    @section Overview

    This script is a resemblence of the Monty Gameshow With The 2 Goats and 1 Car.

    @section Strategy

    The best strategy to win the game is with switching where the probability is 2/3 about 66%.
    Staying only leads to winnin 1/3 about 33%. I think the reason behind this is that first pick
    you mack is only correct 1/3 times, so if Monty will show a Goat door, to build suspense for the
    game show then it means you should probably switch for the car because it would be either hit or miss.
    It makes sense too I would probably be more interested in a show that gave me more emotions.
 */

#include <iostream>
#include <random>
#include <iomanip> 
#include <thread>
#include <mutex>
#include <string>

/**
 * @struct RNG
 * @brief Random number generator wrapper for the Monty Hall simulation.
 *
 * @details The function holds a Mersenne Twister engine (`std::mt19937`) and a uniform
 *          distribution over the doors [0..2].
 * 
 * @var RNG::gen
 * @brief The Mersenne Twister random number engine to help with randomness.
 * 
 * @var RNG::door
 * @brief The uniform distribution for selecting doors [0..2].
 *
 * @note The constructor seeds the random number engine using `std::random_device`
 *       and this is used for better randomness because I was worried I did something wrong.
 * 
 * @note An explicit constructor is provided to allow for seeding the engine.
 */
struct RNG {
    // My own RNG and a [0..2] door distribution
    std::mt19937 gen;
    std::uniform_int_distribution<int> door; // 0..2

    // Seed the engine from random_device and fix for the door range [0..2]
    RNG(): 
        gen(std::random_device{}()),
        door(0, 2) {}
    explicit RNG(uint64_t seed):
        gen(static_cast<std::mt19937::result_type>(seed)), door(0, 2) {}
};

/**
 * @struct Result
 * @brief Holds the results of the Monty Hall simulation.
 * 
 * @var Result::switchWins
 * @brief The number of wins when the player switches doors.
 * 
 * @var Result::stayWins
 * @brief The number of wins when the player stays with their initial choice.
 */
struct Result {
    long long switchWins;
    long long stayWins;
};

/**
 * @brief Simulates a single round of the Monty Hall game where the player always switches.
 *
 * @param rng A reference to an RNG instance for generating random numbers to make sure
 *            everything is random.
 * @return Returns the value 'True' if the player wins by switching, else  `False` otherwise.
 *
 * @details The function randomly places a car behind one of three doors and simulates
 *          the player's initial choice. It then determines which door Monty opens.
 *          If he choose a car door it switches to a goat door randomly and calculates 
 *          whether the player wins by switching to the remaining unopened door.
 * 
 * @note The function uses coin flips to randomly select between the doors if Monty has
 *       chosen a car door.
 */
bool simulate_once_switch_wins(RNG& rng) {
    int car = rng.door(rng.gen);    // Randomly place car behind one of the 3 doors
    int choice = rng.door(rng.gen);  // Random initial player's pick
    
    int opened;
    int switched;
    
    // Monty opens a goat door not chosen by player.
    if (choice == car) {
        // Two goat doors left. Let Monty pick a door randomly UNIFORMLY
        int d1 = (choice + 1) % 3;
        int d2 = (choice + 2) % 3;

        std::uniform_int_distribution<int> coin(0, 1);
        
        // If 0 == choose door 1 else door 2 compared from chosen door
        opened = (coin(rng.gen) == 0) ? d1 : d2;
    } else {
        // Door opened was a GOAT
        opened = 3 - choice - car;
    }

    switched = 3 - choice - opened;
    
    return (switched == car);
};

/**
 * @brief Runs multiple trials of the Monty Hall simulation in a single thread.
 *
 * @param trials The number of trials to run.
 * @param rng A reference to an RNG instance for generating random numbers.
 * @return A `Result` struct containing the number of wins for switching and staying.
 *
 * @details The function iterates through the specified number of trials, simulating
 *          each round using the `simulate_once_switch_wins` function. It counts the
 *          number of wins for both strategies and returns the results.
 * 
 * @details Not USED in the final version but I kept it for reference.
 */
Result run_trials_one_thread(long long trials, RNG& rng) {
    Result result{0, 0};
    for (long long i = 0; i < trials; i++) {
        if (simulate_once_switch_wins(rng)) {
            result.switchWins++;
        } else {
            result.stayWins++;
        } 
    }
    return result;
}

/**
 * @brief Runs multiple trials of the Monty Hall simulation using two threads.
 *
 * @param trials The total number of trials to run.
 * @return A `Result` struct containing the combined number of wins for switching and staying.
 *
 * @details The function divides the total number of trials between two threads, each
 *          running its own instance of the `run_trials_one_thread` function. It uses
 *          separate RNG instances for each thread to ensure randomness. After both
 *          threads complete, it combines their results and returns the total.
 */
Result run_trials_two_threads(long long trials) {
    long long n1 = trials / 2;
    long long n2 = trials - n1;

    std::random_device rd;
    RNG rng1(rd());
    RNG rng2(rd());

    Result r1{0,0}, r2{0,0};

    std::thread t1([&](){ r1 = run_trials_one_thread(n1, rng1); });
    std::thread t2([&](){ r2 = run_trials_one_thread(n2, rng2); });

    t1.join();
    t2.join();

    return Result{ r1.switchWins + r2.switchWins,
                   r1.stayWins   + r2.stayWins };
}

/**
 * @brief Worker function for running trials in a separate thread.
 *
 * @param N The number of trials to run.
 * @param seed A seed for the RNG to ensure different random sequences.
 * @param out A reference to a `Result` struct to store the results of the trials.
 *
 * @details The function initializes an RNG with the provided seed and runs the
 *          specified number of trials using the `run_trials_one_thread` function.
 *          The results are stored in the provided `Result` struct reference.
 * @details NOT USED in the final version but I kept it for reference.
 */
void worker(long long N, uint64_t seed, Result& out) {
    RNG rng(seed);
    out = run_trials_one_thread(N, rng);
}


// Global variables for the locked version
std::mutex mtx;
long long g_switch = 0, g_stay = 0;

/**
 * @brief Worker function for running trials in a separate thread with locking.
 *
 * @param N The number of trials to run.
 * @param seed A seed for the RNG to ensure different random sequences.
 *
 * @details The function initializes an RNG with the provided seed and runs the
 *          specified number of trials. It uses a mutex to safely update global
 *          counters for wins when switching and staying.
 *
 * @details NOT USED in the final version but I kept it for reference.
 */
void worker_locked(long long N, uint64_t seed) {
    RNG rng(seed);
    for (long long i = 0; i < N; i++) {
        bool sw = simulate_once_switch_wins(rng);

        std::lock_guard<std::mutex> lock(mtx);
        if (sw) g_switch++;
        else g_stay++;
    }
}

/**
 * @brief Prints the results of the Monty Hall simulation.
 *
 * @param result A `Result` struct containing the number of wins for switching and staying from the simulation.
 *
 * @details The function calculates the win percentages for both strategies and prints
 *          them to the console with two decimal places of precision using the std::setprecision(2).
 */
void print_result(const Result& result) {
    long long total = result.switchWins + result.stayWins;

    double switchPercentage = 100.0 * static_cast<double>(result.switchWins) / static_cast<double>(total);
    double stayPercentage = 100.0 * static_cast<double>(result.stayWins) / static_cast<double>(total);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Switch would win " << switchPercentage << " percent of experiments." << std::endl;
    std::cout << "Stay would win " << stayPercentage << " percent of experiments." << std::endl;
};


/**
 * @brief The main function for the Monty Hall simulation program.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line argument strings.
 * @return Returns 0 on successful execution, or 1 on error.
 *
 * @details The function checks for the correct number of command-line arguments. It
 *          then parses the number of tests to run, and handles any input errors.
 *          It then runs the Monty Hall simulation using two threads and prints the results to the user.
 */
int main(int argc, char* argv[]) {
    std::cout << "Monty Hall Problem Simulator" << std::endl;


    if (argc != 2) {
        std::cout << "Usage: monty num_tests" << std::endl;
        return 1; 
    }

    // did long long to handle HUGE inputs as well as small 
    long long numTests;
    try {
        numTests = std::stoll(argv[1]);
        if (numTests <= 0) {
            std::cout << "Number of Tests must be positive." << std::endl;
            return 1;
        }
    } catch (const std::invalid_argument&) {
        std::cout << "Number of Test is not a number." << std::endl;
        return 1;
    } catch (const std::out_of_range&) {
        std::cout << "Number of Test is too large of an input." << std::endl;
        return 1;
    }
    
    RNG rng;
    Result result = run_trials_two_threads(numTests);
    print_result(result);
    return 0;
}
