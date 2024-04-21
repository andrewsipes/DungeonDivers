#pragma once
#include "./GameConfig.h"

//here we track the playerStats like score and life
//this is seperate from UI for game config reasons 
//and if additional stats that aren't related are needed

class PlayerStats {

private:
	int score;
	int life;
	int maxhearts;
	int maxscore;
	GameConfig* gameConfig;

public:
	PlayerStats() {
		score = 0000;
		life = 4;
	}

	PlayerStats(GameConfig& _gameConfig) {
		gameConfig = &_gameConfig;
		score = gameConfig->at("Player1").at("score").as<int>();
		life = gameConfig->at("Player1").at("hearts").as<int>();
		maxhearts = gameConfig->at("Player1").at("maxhearts").as<int>();
		maxscore = gameConfig->at("Player1").at("maxscore").as<int>();
	}

	void updateScore(int _score) {
		score += _score;

		if (score > maxscore) {
			score = maxscore;
		}
	}

	void updateLife(int _life) {
		life += _life;

		if (life > maxhearts){
			life = maxhearts;
		}
	}

	int getLife() {
		return life;
	}

	int getScore() {
		return score;
	}

	//update high score if current score is greater than player best
	void saveScore() {
		if (gameConfig->at("Player1").at("highscore").as<int>() > score){
			gameConfig->at("Player1").at("highscore") = score;
		}
	}

};