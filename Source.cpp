#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <time.h>
#include <windows.h>
using namespace std;

//this scrabble program is still under construction
//student name: Marieke van Neutigem
//class group: 2DAE1
//last updated 8-10-16

//to do:
//add better strategy ==> predict possible opponent moves based on proximity within row to special tiles!

//to fix:
//edging problems!!!
//not yet possible to extend words at the end (for the computer)
//there might be score calcuation bugs ==> needs further testing

//possible extras for better playability:
//cut off search after x amount of time and play best word found thus far
//optimize word searching by storing them in data trees
//implement better overall searching algorithm

//function declarations
int getLetterValue(char c);
void fillLetterArr(char letterArr[98][2]);
bool isPossibleCombination(string letters, string ScrabbleWord);
void setUpBLankBoard(char letterArr[15][15][2]);
void drawBoardToConsole(char letterArr[15][15][2], int x);
void fillHand(int NROFLETTERSHAND, int RemainingLetterCount, int nrOfLettersToPick, int NROFLETTERSBAG, string& hand, char LettersInGameArr[98][2]);
void playFirstTurn(int&saveOrient, int&XBoardpos, int&YBoardpos, vector<string>& ScrabbleWordsArr, string& hand, int&  bestWordScore, int& bestWordId, int& iterator);
void playTurn(int&saveOrient, int&XBoardpos, int&YBoardpos, vector<string>& ScrabbleWordsArr, string& hand, int&  bestWordScore, int& bestWordId, int& iterator, int& Orientation, char ScrabbleBoard[][15][2]);
void removeUsedLettersFromHand(vector<string>& ScrabbleWordsArr, int& bestWordId, string& hand);
bool IsValidMove(int& Orientation, string& userWord, char ScrabbleBoard[15][15][2], int& score, int& XboardplayerPos, int& YboardplayerPos, string& lettersOnBoardInWord);

struct player {
	string hand;
	int totalscore;
	bool canMove = true;
};

int main()
{
	//get output handle for changing color in console
	HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
	//open file
	ifstream words;
	words.open("words.txt");
	if (!words.is_open())
	{
		cout << "file was not opened :(\n";
		cin.get();
		return 0;
	}
	//read words to vector
	vector<string> ScrabbleWordsArr;
	while (!words.eof())
	{
		string word;
		getline(words, word);
		//limit word length to board width, can limit it more for shorter load times
		if (word.size() > 2 && word.size() < 16)//filtering out too short words and words with non alphas
		{
			if (word.find('-') == string::npos )
			{
				if (word.find('\'') == string::npos)
				{
					ScrabbleWordsArr.push_back(word);
				}
			}
		}
	}
	words.close();

	//fill bag of letters
	const int NROFLETTERSBAG = 98;
	char LettersInGameArr[NROFLETTERSBAG][2];
	fillLetterArr(LettersInGameArr);
	for (int i = 0; i < NROFLETTERSBAG; ++i)
	{
		LettersInGameArr[i][0] = getLetterValue(LettersInGameArr[i][1]);
	}

	//start game
	player player1;
	player1.hand = "_______";
	player1.totalscore = 0;
	player player2;
	player2.hand = "_______";
	player2.totalscore = 0;

	bool gameOver = false;
	int lettersInHand = 0;
	int RemainingLetterCount = 98;
	const int scrabbleWH = 15;

	//setup empty scrabbleboard for start
	char ScrabbleBoard[scrabbleWH][scrabbleWH][2];
	setUpBLankBoard(ScrabbleBoard);
	cout << "this is a scrabble game.\n";
	drawBoardToConsole(ScrabbleBoard, 0);
	for (int i = 0; i < 15; ++ i)
	{
		for (int j = 0; j < 15; ++j)
		{
			ScrabbleBoard[i][j][1] = '_';
		}
	}

	int XBoardpos = 0;
	int YBoardpos = 0;
	int nrOfLettersToPick = 7;
	srand(unsigned int(time(NULL)));
	bool player1Turn = true;

	bool validanswer = true;
	bool userPlayer = false;
	do {
		string answer;
		cout << "Would you like to play against the computer? enter yes or no and press enter.\nif not the computer will play against itself." << endl;
		cin >> answer;
		if (answer == "yes" || answer == "y")
		{
			validanswer = true;
			userPlayer = true;
		}
		else if (answer == "no" || answer == "n")
		{
			validanswer = true;
			userPlayer = false;
		}
		else
		{
			validanswer = false;
			cout << "invalid answer, please try again.\n";
		}
	} while (!validanswer);

	//gameloop
	while (!gameOver)
	{
		int Orientation = 0;//1 for horizontal, 2 for vertical
		int saveOrient = 0;
		//select your hand
		const int NROFLETTERSHAND = 7;
		if (player1Turn)
		{
			fillHand(NROFLETTERSHAND, RemainingLetterCount, nrOfLettersToPick, NROFLETTERSBAG, player1.hand, LettersInGameArr);
			if (userPlayer)
			{
				cout << "the computer is thinking..." << endl;
			}
		}
		else
		{
			fillHand(NROFLETTERSHAND, RemainingLetterCount, nrOfLettersToPick, NROFLETTERSBAG, player2.hand, LettersInGameArr);
		}

		//select word with highest score
		int iterator = 0;
		int bestWordId = -1;
		int bestWordScore = 0;

		//first turn
		if (ScrabbleBoard[7][7][1] == '_')
		{
			playFirstTurn(saveOrient, XBoardpos, YBoardpos, ScrabbleWordsArr, player1.hand, bestWordScore, bestWordId, iterator);
		}
		//any other turn
		else {
			if (player1Turn)
			{
				//player 1 plays turn
				playTurn(saveOrient, XBoardpos, YBoardpos, ScrabbleWordsArr, player1.hand, bestWordScore, bestWordId, iterator, Orientation, ScrabbleBoard);
			}
			else 
			{
				if (!userPlayer)
				{
					//player 2 plays turn
					playTurn(saveOrient, XBoardpos, YBoardpos, ScrabbleWordsArr, player2.hand, bestWordScore, bestWordId, iterator, Orientation, ScrabbleBoard);
				}
				else
				{
					//user plays trurn
					bool invalidInput = false;
					do {
						bool invalid = false;
						string playerWord;
						string playerOrientationStr;
						string XboardplayerPosStr;
						string YboardplayerPosStr;
						cout << "your letters: " << player2.hand << ",   enter -1 0 0 0 if you can\'t find any words" << endl;
						cin >> playerWord >> playerOrientationStr >> XboardplayerPosStr >> YboardplayerPosStr;

						int playerOrient;
						if (playerOrientationStr == "horizontal")
						{
							playerOrient = 1;
						}
						else if (playerOrientationStr == "vertical")
						{
							playerOrient = 2;
						}
						else
						{
							invalid = true;
						}


						int XboardplayerPos = stoi(XboardplayerPosStr);//cant find a saver alternative for stoi thus far...
						int YboardplayerPos = stoi(YboardplayerPosStr);
						if (XboardplayerPos < 0 || XboardplayerPos > 14 || YboardplayerPos < 0 || YboardplayerPos > 14)
						{
							invalidInput = true;
						}

						//check if real word
						int iterateCheck = 0;
						if (!invalid)
						{
							for (string ScrabbleWord : ScrabbleWordsArr)
							{
								if (playerWord == ScrabbleWord)
								{
									bestWordId = iterateCheck;
								}
								++iterateCheck;
							}
						}
						if (bestWordId == -1)
						{
							invalid = true;
						}

						if (invalid)
						{
							cout << "invalid input please retry.\n";
						}
						else if (playerWord != "-1")
						{
							string lettersOnBoardInWord = "";
							//check what score is achieved and check any overlap
							int score = 0;
							if (!IsValidMove(playerOrient, playerWord, ScrabbleBoard, score, XboardplayerPos, YboardplayerPos, lettersOnBoardInWord))
							{
								invalidInput = true;
								cout << "invalid move! please retry\n";
							}
							//check if actually made up of letters in hand + board
							else if (isPossibleCombination( lettersOnBoardInWord + player2.hand, playerWord) && lettersOnBoardInWord.size() > 0)
							{
								XBoardpos = XboardplayerPos;
								YBoardpos = YboardplayerPos;
								saveOrient = playerOrient;
								bestWordScore = score;
							}
							else invalid = true;
						}
						//means player couldnt place a word
						if (playerWord == "-1")
						{
							invalid = false;
							bestWordId = -1;
						}
						else if (invalid) invalidInput = true;
					} while(invalidInput);
					
				}
			}
		}

		gameOver = true;
		RemainingLetterCount = 0;
		for (int i = 0; i < NROFLETTERSBAG; ++i)
		{
			if (LettersInGameArr[i][1] != '_')
			{
				gameOver = false;
				++RemainingLetterCount;
			}
		}

		if (bestWordId > 0)
		{
			string bestWord = string(ScrabbleWordsArr[bestWordId]);
			nrOfLettersToPick = bestWord.size();

			//for debugging:
			//cerr << ScrabbleWordsArr[bestWordId] << " : " << bestWordScore << " : " << bestWordId << endl;

			//load best word onto the board in its given position
			for (unsigned int i = 0; i < bestWord.size(); ++i)
			{
				if (saveOrient == 1)
				{
					if (ScrabbleBoard[XBoardpos + i][YBoardpos][1] == '_')
					{
						ScrabbleBoard[XBoardpos + i][YBoardpos][1] = bestWord[i];
					}
				}
				else if (saveOrient == 2)
				{
					if (ScrabbleBoard[XBoardpos][YBoardpos + i][1] == '_')
					{
						ScrabbleBoard[XBoardpos][YBoardpos + i][1] = bestWord[i];
					}
				}
			}
			
			//take used letters out of hand
			if (player1Turn)
			{
				removeUsedLettersFromHand(ScrabbleWordsArr, bestWordId, player1.hand);
				player1.totalscore += bestWordScore;
			}
			else
			{
				removeUsedLettersFromHand(ScrabbleWordsArr, bestWordId, player2.hand);
				player2.totalscore += bestWordScore;
			}

			lettersInHand = NROFLETTERSHAND - ScrabbleWordsArr[bestWordId].size();
		}
		//else gameOver = true;//i forgot why this was here...
		
		//draw scrabbleboard
		system("cls");//should avoid using system function, find better solution..
		if (userPlayer)
		{
			cout << "totalScore the computer : " << player1.totalscore << endl;
			cout << "totalScore user : " << player2.totalscore << endl;
		}
		else
		{
			cout << "totalScore player 1: " << player1.totalscore << endl;
			cout << "totalScore player 2: " << player2.totalscore << endl;
		}

		//cycle through all the tiles
		for (int i = 0; i < 15; ++i)
		{
			if (i == 0)
			{
				//first set up the numbers on the top
				SetConsoleTextAttribute(hstdout, FOREGROUND_RED);
				cout << "     0 1 2 3 4 5 6 7 8 9 1011121314\n";
			}
			for (int j = 0; j < 15; ++j)
			{
				if (j == 0)
				{
					//set up the numbers on the side
					SetConsoleTextAttribute(hstdout, FOREGROUND_RED);
					cout << i << "]  ";
					if (i < 10)
					{
						cout << " ";
					}
				}
				if (ScrabbleBoard[j][i][1] != '_')
				{
					//draw any letters onto the board
					SetConsoleTextAttribute(hstdout, FOREGROUND_BLUE | BACKGROUND_GREEN);//adds colour to make it more readable
					cout << ScrabbleBoard[j][i][1] << " ";
				}
				else
				{
					//draw the board itself
					SetConsoleTextAttribute(hstdout, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);//sets colours back to black background and white foreground
					cout << ScrabbleBoard[j][i][0] << " ";
				}
			}
			cout << endl;
		}
		//means that player wasn't able to place a word
		if (bestWordId == -1)
		{
			if (player1Turn)
			{
				player1.canMove = false;
			}
			else
			{
				player2.canMove = false;
			}
		}
		else
		{
			if (player1Turn)
			{
				player1.canMove = true;
			}
			else
			{
				player2.canMove = true;
			}
			if (userPlayer && player1Turn)
			{
				cout << "the computer played: " << ScrabbleWordsArr[bestWordId] << " for " << bestWordScore << " points." << endl;
			}
			else
			{
				cout << "word played: " << ScrabbleWordsArr[bestWordId] << " for " << bestWordScore << " points." << endl;
			}
		}
		if (!player1.canMove && !player2.canMove)
		{
			gameOver = true;
			cout << "both player are unable to make any more words.\n";
		}
		player1Turn = !player1Turn;
		if (userPlayer == true && !player1Turn)
		{
			cout << "please enter the word you would like to play followed by the direction (horizontal or vertical)" << endl;
			cout << "and the coordinates of the first letter on the board, do so in the follow layout without capitals:"<< endl << "word horizontal 4 5" << endl;
		}
	}

	cout << "Game Over!\nplayer 1 scored " << player1.totalscore << " points,\nplayer 2 scored: " << player2.totalscore << " points.\n";
	if (!player1.canMove && !player2.canMove)
	{
		cout << "both player are unable to make any more words.\n";
	}
	if (player1.totalscore > player2.totalscore)
	{
		cout << "Player 1 won!" << endl;
	}
	else if (player1.totalscore < player2.totalscore)
	{
		cout << "Player 2 won!" << endl;
	}
	else
	{
		cout << "It's a tie!" << endl;
	}

	cin.ignore();
	cin.get();
	return 0;
}

bool IsValidMove(int& Orientation, string& userWord, char ScrabbleBoard[15][15][2], int& score, int& XboardplayerPos, int& YboardplayerPos, string& lettersOnBoardInWord)
{
	string onBoard = "";
	for (char v : userWord)
	{
			score += getLetterValue(v);
	}
	for (unsigned int r = 0; r < userWord.size(); ++r)
	{
		if (Orientation == 1)
		{
			if (ScrabbleBoard[XboardplayerPos + r][YboardplayerPos][1] != '_')
			{
				onBoard += ScrabbleBoard[XboardplayerPos + r][YboardplayerPos][1];
			}
			//add special tile score
			if (ScrabbleBoard[XboardplayerPos + r][YboardplayerPos][0] != '_' && ScrabbleBoard[XboardplayerPos + r][YboardplayerPos][1] == '_')
			{
				switch (ScrabbleBoard[XboardplayerPos + r][YboardplayerPos][0]) {
				case '2':
					score += getLetterValue(userWord[r]);
					break;
				case '3':
					score += 2 * getLetterValue(userWord[r]);
					break;
				case '4':
					score *= 2;
					break;
				case '6':
					score *= 3;
					break;
				};
			}
			if (ScrabbleBoard[XboardplayerPos + r][YboardplayerPos][1] == '_')
			{
				//keep word from edging onto other word
				//buggy...
				if (ScrabbleBoard[XboardplayerPos + r][YboardplayerPos + 1][1] != '_' || ScrabbleBoard[XboardplayerPos + r][YboardplayerPos - 1][1] != '_')
				{
					score = 0;
				}
			}
			else if (ScrabbleBoard[XboardplayerPos + r][YboardplayerPos][1] != userWord[r])//keep from using letters on the board not in the word
			{
				return false;
			}
			if (XboardplayerPos + r > 14 || XboardplayerPos + r < 0)//keep from going off the board
			{
				return false;
			}
			if (r == 0 && ScrabbleBoard[XboardplayerPos - 1][YboardplayerPos][1] != '_')
			{
				return false;
			}
			if (r == userWord.size() - 1 && ScrabbleBoard[XboardplayerPos + r + 1][YboardplayerPos][1] != '_')
			{
				return false;
			}
		}
		else if (Orientation == 2)
		{
			if (ScrabbleBoard[XboardplayerPos][YboardplayerPos + r][1] != '_')
			{
				onBoard += ScrabbleBoard[XboardplayerPos][YboardplayerPos + r][1];
			}
			if (ScrabbleBoard[XboardplayerPos][YboardplayerPos + r][0] != '_' && ScrabbleBoard[XboardplayerPos][YboardplayerPos + r][1] == '_')
			{
				//add special tile score
				switch (ScrabbleBoard[XboardplayerPos][YboardplayerPos + r][0]) {
				case '2':
					score += getLetterValue(userWord[r]);
					break;
				case '3':
					score += 2 * getLetterValue(userWord[r]);
					break;
				case '4':
					score *= 2;
					break;
				case '6':
					score *= 3;
					break;
				};
			}
			if (ScrabbleBoard[XboardplayerPos][YboardplayerPos + r][1] == '_')
			{
				//keep word from edging onto other word
				//buggy...
				if (ScrabbleBoard[XboardplayerPos + 1][YboardplayerPos + r][1] != '_' || ScrabbleBoard[XboardplayerPos - 1][YboardplayerPos + r][1] != '_')
				{
					return false;
				}

			}
			else if (ScrabbleBoard[XboardplayerPos][YboardplayerPos + r][1] != userWord[r])//keep from using letters on the board not in the word
			{
				return false;
			}
			if (YboardplayerPos + r > 14 || YboardplayerPos + r < 0)//keep from going off the board
			{
				return false;
			}
			if (r == 0 && ScrabbleBoard[XboardplayerPos][YboardplayerPos - 1][1] != '_')
			{
				return false;
			}
			if (r == userWord.size() - 1 && ScrabbleBoard[XboardplayerPos][YboardplayerPos + r + 1][1] != '_')
			{
				return false;
			}
		}
	}
	lettersOnBoardInWord = onBoard;
	return true;
}

void removeUsedLettersFromHand(vector<string>& ScrabbleWordsArr, int& bestWordId, string& hand)
{
	//take used letters from hand 
	for (char c : ScrabbleWordsArr[bestWordId])
	{
		for (unsigned int i = 0; i < hand.size(); ++i)
		{
			if ((hand[i] == c || hand[i] + 32 == c))
			{
				hand[i] = '_';
			}
		}
	}
}

void playTurn(int&saveOrient, int&XBoardpos, int&YBoardpos, vector<string>& ScrabbleWordsArr, string& hand, int&  bestWordScore, int& bestWordId, int& iterator, int& Orientation, char ScrabbleBoard[15][15][2])
{
	//this seriously needs optimizing....
	for (int i = 0; i < 15; ++i)
	{
		for (int j = 0; j < 15; ++j)
		{
			if (ScrabbleBoard[i][j][1] != '_')
			{
				if (ScrabbleBoard[i - 1][j][1] == '_' && ScrabbleBoard[i + 1][j][1] == '_')
				{
					Orientation = 1;
				}
				else if (ScrabbleBoard[i][j - 1][1] == '_' && ScrabbleBoard[i][j + 1][1] == '_')
				{
					Orientation = 2;
				}
				else
				{
					Orientation = 0;
				}
				char LetterOnBoard = ScrabbleBoard[i][j][1];
				iterator = 0;
				for (string ScrabbleWord : ScrabbleWordsArr)
				{
					for (char c : ScrabbleWord)
					{
						if (c == LetterOnBoard)
						{
							if (isPossibleCombination(hand + LetterOnBoard, ScrabbleWord))
							{
								int tempXboardPos = 0;
								int tempYboardPos = 0;
								int index = 0;
								int d = 0;
								for (char c : ScrabbleWord)
								{
									//get index of letter in word
									if (c == LetterOnBoard)
									{
										//to fix: if the letter is in the word more than once choose the version with the highest score
										index = d;
									}
									++d;
								}
								if (Orientation == 1)
								{
									tempXboardPos = i - index;
									tempYboardPos = j;

								}
								else if (Orientation == 2)
								{
									tempYboardPos = j - index;
									tempXboardPos = i;
								}

								int score = 0;
								for (char v : ScrabbleWord)
								{
									//if (v != c)
									//{
										score += getLetterValue(v);
									//}
								}
								for (unsigned int r = 0; r < ScrabbleWord.size(); ++r)
								{
									if (Orientation == 1)
									{
										//add special tile score
										if (ScrabbleBoard[tempXboardPos + r][tempYboardPos][0] != '_' && ScrabbleBoard[tempXboardPos + r][tempYboardPos][1] == '_')
										{
											switch (ScrabbleBoard[tempXboardPos + r][tempYboardPos][0]) {
											case '2':
												score += getLetterValue(ScrabbleWord[r]);
												break;
											case '3':
												score += 2 * getLetterValue(ScrabbleWord[r]);
												break;
											case '4':
												score *= 2;
												break;
											case '6':
												score *= 3;
												break;
											};
										}
										if (ScrabbleBoard[tempXboardPos + r][tempYboardPos][1] == '_')
										{
											//keep word from edging onto other word
											//buggy...
											if (ScrabbleBoard[tempXboardPos + r][tempYboardPos + 1][1] != '_' || ScrabbleBoard[tempXboardPos + r][tempYboardPos - 1][1] != '_')
											{
												score = 0;
											}
										}
										else if (ScrabbleBoard[tempXboardPos + r][tempYboardPos][1] != ScrabbleWord[r])//keep from using letters on the board not in the word
										{
											score = 0;
										}
										if (tempXboardPos + r > 14 || tempXboardPos + r < 0)//keep from going off the board
										{
											score = 0;
										}
										if (r == 0 && ScrabbleBoard[tempXboardPos - 1][tempYboardPos][1] != '_')
										{
											score = 0;
										}
										if (r == ScrabbleWord.size() - 1 && ScrabbleBoard[tempXboardPos + r + 1][tempYboardPos][1] != '_')
										{
											score = 0;
										}
									}
									else if (Orientation == 2)
									{
										if (ScrabbleBoard[tempXboardPos][tempYboardPos + r][0] != '_' && ScrabbleBoard[tempXboardPos][tempYboardPos + r][1] == '_')
										{
											//add special tile score
											switch (ScrabbleBoard[tempXboardPos][tempYboardPos + r][0]) {
											case '2':
												score += getLetterValue(ScrabbleWord[r]);
												break;
											case '3':
												score += 2 * getLetterValue(ScrabbleWord[r]);
												break;
											case '4':
												score *= 2;
												break;
											case '6':
												score *= 3;
												break;
											};
										}
										if (ScrabbleBoard[tempXboardPos][tempYboardPos + r][1] == '_')
										{
											//keep word from edging onto other word
											//buggy...
											if (ScrabbleBoard[tempXboardPos + 1][tempYboardPos + r][1] != '_' || ScrabbleBoard[tempXboardPos - 1][tempYboardPos + r][1] != '_')
											{
												score = 0;
											}

										}
										else if (ScrabbleBoard[tempXboardPos][tempYboardPos + r][1] != ScrabbleWord[r])//keep from using letters on the board not in the word
										{
											score = 0;
										}
										if (tempYboardPos + r > 14 || tempYboardPos + r < 0)//keep from going off the board
										{
											score = 0;
										}
										if (r == 0 && ScrabbleBoard[tempXboardPos][tempYboardPos - 1][1] != '_')
										{
											score = 0;
										}
										if (r == ScrabbleWord.size() - 1 && ScrabbleBoard[tempXboardPos][tempYboardPos + r + 1][1] != '_')
										{
											score = 0;
										}
									}
								}
								//load in if higher score
								if (score > bestWordScore)
								{
									//store the word
									if (Orientation != 0)
									{
										XBoardpos = tempXboardPos;
										YBoardpos = tempYboardPos;
										saveOrient = Orientation;
										bestWordScore = score;
										bestWordId = iterator;

									}
								}
							}
						}
					}
					++iterator;
				}
			}
		}
	}//too many brackets!!! clean up!!!
}

void playFirstTurn(int&saveOrient, int&XBoardpos, int&YBoardpos, vector<string>& ScrabbleWordsArr, string& hand, int&  bestWordScore, int& bestWordId, int& iterator)
{
	//set default starting position
	saveOrient = 1;
	XBoardpos = 7;
	YBoardpos = 7;
	for (string ScrabbleWord : ScrabbleWordsArr)
	{
		if (isPossibleCombination(hand, ScrabbleWord))
		{
			int score = 0;
			for (char c : ScrabbleWord)
			{
				score += getLetterValue(c);
			}
			if (score > bestWordScore)
			{
				bestWordScore = score;
				bestWordId = iterator;
			}
		}
		++iterator;
	}
}

void fillHand(int NROFLETTERSHAND, int RemainingLetterCount, int nrOfLettersToPick, int NROFLETTERSBAG, string& hand, char LettersInGameArr[98][2])
{
	for (int i = 0; i < NROFLETTERSHAND; ++i)
	{
		if (RemainingLetterCount > nrOfLettersToPick)//instead of seven should be nr of letters to pick
		{
			int N = rand() % NROFLETTERSBAG;
			bool noSucces = false;
			do
			{
				//fill in dashes for removed letters from bag
				if (LettersInGameArr[N][1] != '_')
				{
					if (hand[i] == '_')
					{
						hand[i] = LettersInGameArr[N][1];
						LettersInGameArr[N][1] = '_';
						LettersInGameArr[N][0] = '_';
					}
					noSucces = false;
				}
				else
				{
					noSucces = true;
					N = rand() % NROFLETTERSBAG;
				}
			} while (noSucces);
		}
		else
		{
			for (int p = 0; p < NROFLETTERSBAG; ++p)
			{
				if (LettersInGameArr[p][1] != '_')
				{
					if (hand[i] == '_')
					{
						hand[i] = LettersInGameArr[p][1];
					}
				}
			}
		}
	}
	//cerr << hand << endl;
}

void drawBoardToConsole(char letterArr[15][15][2], int x)
{
	for (int i = 0; i < 15; ++i)
	{
		if (i == 0)
		{ 
			cout << "     0 1 2 3 4 5 6 7 8 9 1011121314\n";
		}
		for (int j = 0; j < 15; ++j)
		{
			if (j == 0)
			{
				cout << i << "]  ";
				if (i < 10)
				{
					cout << " ";
				}
			}
			cout << letterArr[j][i][0] << " ";
		}
		cout << endl;
	}
}

void setUpBLankBoard(char letterArr[15][15][2])
{
	for (int i = 0; i < 15; ++i)
	{
		for (int j = 0; j < 15; ++j)
		{
			letterArr[i][j][0] = '_';
		}
	}

	letterArr[0][0][0] = '6'; // 6 means triple word value
	letterArr[7][0][0] = '6'; 
	letterArr[14][0][0] = '6'; 
	letterArr[0][7][0] = '6'; 
	letterArr[14][7][0] = '6'; 
	letterArr[0][14][0] = '6';
	letterArr[7][14][0] = '6';
	letterArr[14][14][0] = '6';

	letterArr[1][1][0] = '4';// 4 means double word value
	letterArr[2][2][0] = '4';
	letterArr[3][3][0] = '4';
	letterArr[4][4][0] = '4';
	letterArr[5][5][0] = '4';
	letterArr[6][6][0] = '4';
	letterArr[8][8][0] = '4';
	letterArr[9][9][0] = '4';
	letterArr[10][10][0] = '4';
	letterArr[11][11][0] = '4';
	letterArr[12][12][0] = '4';
	letterArr[13][13][0] = '4';

	letterArr[13][1][0] = '4';
	letterArr[12][2][0] = '4';
	letterArr[11][3][0] = '4';
	letterArr[10][4][0] = '4';
	letterArr[9][5][0] = '4';
	letterArr[8][6][0] = '4';
	letterArr[6][8][0] = '4';
	letterArr[5][9][0] = '4';
	letterArr[4][10][0] = '4';
	letterArr[3][11][0] = '4';
	letterArr[2][12][0] = '4';
	letterArr[1][13][0] = '4';

	letterArr[5][1][0] = '3'; // 3 means triple letter value
	letterArr[9][1][0] = '3';
	letterArr[1][5][0] = '3';
	letterArr[5][5][0] = '3';
	letterArr[9][5][0] = '3';
	letterArr[13][5][0] = '3';
	letterArr[1][9][0] = '3';
	letterArr[5][9][0] = '3';
	letterArr[9][9][0] = '3';
	letterArr[13][9][0] = '3';
	letterArr[5][13][0] = '3';
	letterArr[9][13][0] = '3';

	letterArr[3][0][0] = '2'; // 2 means double letter value
	letterArr[11][0][0] = '2';
	letterArr[6][2][0] = '2';
	letterArr[8][2][0] = '2';
	letterArr[0][3][0] = '2';
	letterArr[7][3][0] = '2';
	letterArr[14][3][0] = '2';
	letterArr[2][6][0] = '2';
	letterArr[6][6][0] = '2';
	letterArr[8][6][0] = '2';
	letterArr[12][6][0] = '2';
	letterArr[3][7][0] = '2';
	letterArr[11][7][0] = '2';
	letterArr[2][8][0] = '2';
	letterArr[6][8][0] = '2';
	letterArr[8][8][0] = '2';
	letterArr[12][8][0] = '2';
	letterArr[0][11][0] = '2';
	letterArr[7][11][0] = '2';
	letterArr[14][11][0] = '2';
	letterArr[6][12][0] = '2';
	letterArr[8][12][0] = '2';
	letterArr[3][14][0] = '2';
}

bool isPossibleCombination(string letters, string ScrabbleWord)
{
		if (ScrabbleWord.size() <= letters.size())
		{
			string ScrabbleWordCopy = ScrabbleWord;
			int count = 0;
			string lettersCopy = letters;
			//start looping through the characters
			for (unsigned int j = 0; j < ScrabbleWordCopy.size(); ++j)
			{
				for (unsigned int i = 0; i < lettersCopy.size(); ++i)
				{
					if ((lettersCopy[i] == ScrabbleWordCopy[j] || lettersCopy[i] + 32 == ScrabbleWordCopy[j] || lettersCopy[i] - 32 == ScrabbleWordCopy[j]) && lettersCopy[i] != '_')
					{
						++count;
						lettersCopy[i] = '_';
						ScrabbleWordCopy[j] = '_';
					}
				}
			}
			//if all characters where found return true
			if (count == ScrabbleWordCopy.size())
			{
				return true;
			}
		}
		return false;
}

int getLetterValue(char c)
{
	switch (c) {
	case 'a': case 'A': return 1; break;
	case 'b': case 'B': return 3; break;
	case 'c': case 'C': return 3; break;
	case 'd': case 'D': return 2; break;
	case 'e': case 'E': return 1; break;
	case 'f': case 'F': return 4; break;
	case 'g': case 'G': return 2; break;
	case 'h': case 'H': return 4; break;
	case 'i': case 'I': return 1; break;
	case 'j': case 'J': return 8; break;
	case 'k': case 'K': return 5; break;
	case 'l': case 'L': return 1; break;
	case 'm': case 'M': return 3; break;
	case 'n': case 'N': return 1; break;
	case 'o': case 'O': return 1; break;
	case 'p': case 'P': return 3; break;
	case 'q': case 'Q': return 10; break;
	case 'r': case 'R': return 1; break;
	case 's': case 'S': return 1; break;
	case 't': case 'T': return 1; break;
	case 'u': case 'U': return 1; break;
	case 'v': case 'V': return 4; break;
	case 'w': case 'W': return 4; break;
	case 'x': case 'X': return 8; break;
	case 'y': case 'Y': return 4; break;
	case 'z': case 'Z': return 10; break;
	}
	return -1;
}

void fillLetterArr(char letterArr[98][2])
{
	const int numA = 9;
	const int numB = 2;
	const int numC = 2;
	const int numD = 4;
	const int numE = 12;
	const int numF = 2;
	const int numG = 3;
	const int numH = 2;
	const int numI = 9;
	const int numJ = 1;
	const int numK = 1;
	const int numL = 4;
	const int numM = 2;
	const int numN = 6;
	const int numO = 8;
	const int numP = 2;
	const int numQ = 1;
	const int numR = 6;
	const int numS = 4;
	const int numT = 6;
	const int numU = 4;
	const int numV = 2;
	const int numW = 2;
	const int numX = 1;
	const int numY = 2;
	const int numZ = 1;
	
	int counter = 0;
	for (int i = 0; i < numA; ++i)
	{
		letterArr[counter][1] = 'A';
		counter++;
	}
	for (int i = 0; i < numB; ++i)
	{
		letterArr[counter][1] = 'B';
		counter++;
	}
	for (int i = 0; i < numC; ++i)
	{
		letterArr[counter][1] = 'C';
		counter++;
	}
	for (int i = 0; i < numE; ++i)
	{
		letterArr[counter][1] = 'E';
		counter++;
	}
	for (int i = 0; i < numF; ++i)
	{
		letterArr[counter][1] = 'F';
		counter++;
	}
	for (int i = 0; i < numA; ++i)
	{
		letterArr[counter][1] = 'G';
		counter++;
	}
	for (int i = 0; i < numA; ++i)
	{
		letterArr[counter][1] = 'H';
		counter++;
	}
	for (int i = 0; i < numA; ++i)
	{
		letterArr[counter][1] = 'I';
		counter++;
	}
	for (int i = 0; i < numA; ++i)
	{
		letterArr[counter][1] = 'J';
		counter++;
	}
	for (int i = 0; i < numA; ++i)
	{
		letterArr[counter][1] = 'K';
		counter++;
	}
	for (int i = 0; i < numL; ++i)
	{
		letterArr[counter][1] = 'L';
		counter++;
	}
	for (int i = 0; i < numM; ++i)
	{
		letterArr[counter][1] = 'M';
		counter++;
	}
	for (int i = 0; i < numN; ++i)
	{
		letterArr[counter][1] = 'N';
		counter++;
	}
	for (int i = 0; i < numO; ++i)
	{
		letterArr[counter][1] = 'O';
		counter++;
	}
	for (int i = 0; i < numP; ++i)
	{
		letterArr[counter][1] = 'P';
		counter++;
	}
	for (int i = 0; i < numQ; ++i)
	{
		letterArr[counter][1] = 'Q';
		counter++;
	}
	for (int i = 0; i < numR; ++i)
	{
		letterArr[counter][1] = 'R';
		counter++;
	}
	for (int i = 0; i < numS; ++i)
	{
		letterArr[counter][1] = 'S';
		counter++;
	}
	for (int i = 0; i < numT; ++i)
	{
		letterArr[counter][1] = 'T';
		counter++;
	}
	for (int i = 0; i < numU; ++i)
	{
		letterArr[counter][1] = 'U';
		counter++;
	}
	for (int i = 0; i < numV; ++i)
	{
		letterArr[counter][1] = 'V';
		counter++;
	}
	for (int i = 0; i < numW; ++i)
	{
		letterArr[counter][1] = 'W';
		counter++;
	}
	for (int i = 0; i < numX; ++i)
	{
		letterArr[counter][1] = 'X';
		counter++;
	}
	for (int i = 0; i < numY; ++i)
	{
		letterArr[counter][1] = 'Y';
		counter++;
	}
	for (int i = 0; i < numZ; ++i)
	{
		letterArr[counter][1] = 'Z';
		counter++;
	}
}
