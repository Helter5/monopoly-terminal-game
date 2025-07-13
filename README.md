## Monopoly Terminal Game

A simplified Monopoly board game implemented in C, playable in the terminal.

### Features
- Supports 2-4 players
- Property buying, rent, and color monopolies
- Jail, Free Parking, Go To Jail, and Jail Pass mechanics
- Command-line options for custom output
- Automated test suite with sample scenarios

### Project Structure
- `src/` — Game logic and main program (`z4.c`, `monopoly.c`)
- `include/` — Header file (`monopoly.h`)
- `tests/` — Test runner and test cases
- `CMakeLists.txt` — Build configuration
- `tester.py` — Python script for running tests

### Running the Game
Run the game with the desired number of players (2-4):

```sh
./z4 -n <number_of_players> [options]
```

**Options:**
- `-s` — Print the game board after each round
- `-p` — Print player details after each round
- `-g` — Print both player details and game board after each round

Example:
```sh
./z4 -n 3 -s
```

The game expects moves (dice rolls) as input from stdin, one per line.

### Testing
Automated tests are provided in the `tests/` directory. To run all tests:

```sh
python3 tester.py
```

Test cases are organized in subfolders (`s1`, `s2`, ...), each containing:
- `stdin_XX.txt` — Input moves
- `stdout_XX.txt` — Expected output
- `parameters_XX.txt` — Command-line arguments for the test
