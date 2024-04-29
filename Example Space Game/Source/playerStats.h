#pragma once
#include "./GameConfig.h"

//here we track the playerStats like score and life
//this is seperate from UI for game config reasons 
//and if additional stats that aren't related are needed

class PlayerStats {

private:
	int score, scoreBeforeDeath, maxScore;
	int hearts, heartsBeforeDeath, maxHearts;
	GameConfig* gameConfig;

public:
	//some default values, we shouldn't use this
	PlayerStats() {
		score = 0000;
		hearts = 4;
	}

	//sets all required parementers when constructed
	PlayerStats(GameConfig& _gameConfig) {
		gameConfig = &_gameConfig;
		score = gameConfig->at("Player1").at("score").as<int>();
		hearts = gameConfig->at("Player1").at("hearts").as<int>();
		maxHearts = gameConfig->at("Player1").at("maxhearts").as<int>();
		maxScore = gameConfig->at("Player1").at("maxscore").as<int>();
		heartsBeforeDeath = hearts;
		scoreBeforeDeath = score;
	}

	//updates the player score, accounts for max score which 9999
	void updateScore(int _score) {
		score += _score;

		if (score > maxScore) {
			score = maxScore;
		}
	}

	//updates the player score, accounts for max score which 9999
	void setScore(int _score) {
		score = _score;

		if (score > maxScore) {
			score = maxScore;
		}
	}

	//for saving score at the beginning of the level
	void updateScoreBeforeDeath() {
		scoreBeforeDeath = score;
	}

	//for saving heath at the beginning of the level
	void updateHeartsBeforeDeath(){
		heartsBeforeDeath = hearts;
	}

	//resets player after death
	//score will be the same as the start of the level
	void restartPlayer() {

		score = scoreBeforeDeath;
		hearts = heartsBeforeDeath;
	}

	//update player health
	void updateHearts(int _hearts) {
		hearts += _hearts;

		if (hearts > maxHearts){
			hearts = maxHearts;
		}
	}

	//update player health
	void setHearts(int _hearts) {
		hearts = _hearts;

		if (hearts > maxHearts) {
			hearts = maxHearts;
		}
	}

	//get player health value
	int getHearts() {
		return hearts;
	}

	//get player score value
	int getScore() {
		return score;
	}

	//get player health value
	int getHeartsBeforeDeath() {
		return heartsBeforeDeath;
	}

	//get player score value
	int getScoreBeforeDeath() {
		return scoreBeforeDeath;
	}

	//update high score if current score is greater than player best
	void saveScore() {
		if (gameConfig->at("Player1").at("highscore").as<int>() > score){
			gameConfig->at("Player1").at("highscore") = score;
		}
	}

};