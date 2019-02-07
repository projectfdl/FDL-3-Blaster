/*****************************************************************
MicroView_Analog_Clock.ino
SFE_LSM9DS0 Library Simple Example Code
Original Creation Date: February 28, 2015
https://github.com/sparkfun/MicroView/tree/master/Libraries

Play pong on your MicroView! Tie a potentiometer to A1 for
player 1 and A2 for player 2 (optional).

If you want to play against the computer, uncomment out
the SINGLE_PLAYER define on line 26.

Development environment specifics:
	IDE: Arduino 1.6.0
	Hardware Platform: MicroView

Please see the LICENSE file included with this sketch. This
software was based on MicroPong by Shane Lynch:
https://github.com/iequalshane/MicroPong
*****************************************************************/
#include <MicroView.h>

// Uncomment out this next line if you want to play against
// an AI:
//#define SINGLE_PLAYER

// Pins used to control player paddles. These can be any
// available analog input pin:
const int player1Pin = A1;
#ifndef SINGLE_PLAYER
const int player2Pin = A2;
#endif
const float sensorMaxValue = 1024.0;

const int renderDelay = 16; // About 60hz
const int startDelay = 4000;
const int gameOverDelay = 6000;

const int scoreToWin = 10;
int player1Score = 0;
int player2Score = 0;

const float paddleWidth = LCDWIDTH / 16.0;
const float paddleHeight = LCDHEIGHT / 3.0;
const float halfPaddleWidth = paddleWidth / 2.0;
const float halfPaddleHeight = paddleHeight / 2.0;

float player1PosX = 1.0 + halfPaddleWidth;
float player1PosY = 0.0;
float player2PosX = LCDWIDTH - 1.0 - halfPaddleWidth;
float player2PosY = 0.0;

// This is only used in SINGLE_PLAYER mode:
#ifdef SINGLE_PLAYER
float enemyVelY = 0.5;
#endif

const float ballRadius = 2.0;
const float ballSpeedX = 1.0;
float ballPosX = LCDWIDTH / 2.0;
float ballPosY = LCDHEIGHT / 2.0;
float ballVelX = -1.0 * ballSpeedX;
float ballVelY = 0;

void setup()
{
  initializeGraphics();
  initializeInput();
  displayGameStart();
}

void resetGame()
{
  player2Score = 0;
  player1Score = 0;
  player2PosY = 0.0;
  ballPosX = LCDWIDTH / 2.0;
  ballPosY = LCDHEIGHT / 2.0;
  ballVelX = -1.0 * ballSpeedX;
  ballVelY = 0.0;
}

void initializeGraphics()
{
  uView.begin();
  uView.setFontType(1);
}

void initializeInput()
{
  digitalWrite(player1Pin, HIGH);
  pinMode(player1Pin, INPUT);
  digitalWrite(player2Pin, HIGH);
  pinMode(player2Pin, INPUT);
}

void displayGameStart()
{
  uView.clear(PAGE);
  renderString(20, 10, "Get");
  renderString(10, 30, "Ready!");
  uView.display();
  delay(startDelay);
}

void loop()
{
  updateGame();
  renderGame();

  if (player1Score >= scoreToWin)
  {
    gameOver(true);
  }
  else if (player2Score >= scoreToWin)
  {
    gameOver(false);
  }
}

void updateGame()
{
  updatePlayer1();
  updatePlayer2();
  updateBall();
}

float clampPaddlePosY(float paddlePosY)
{
  float newPaddlePosY = paddlePosY;

  if (paddlePosY - halfPaddleHeight < 0)
  {
    newPaddlePosY = halfPaddleHeight;
  }
  else if (paddlePosY + halfPaddleHeight > LCDHEIGHT)
  {
    newPaddlePosY = LCDHEIGHT - halfPaddleHeight;
  }

  return newPaddlePosY;
}

void updatePlayer1()
{
  float knobValue = 1 - analogRead(player1Pin) / sensorMaxValue;
  player1PosY = clampPaddlePosY(knobValue * LCDHEIGHT);
}

void updatePlayer2()
{
  // If it's a single player game, update the AI's position
#ifdef SINGLE_PLAYER
  // Follow the ball at a set speed
  if (player2PosY < ballPosY)
  {
    player2PosY += enemyVelY;
  }
  else if (player2PosY > ballPosY)
  {
    player2PosY -= enemyVelY;
  }

  player2PosY = clampPaddlePosY(player2PosY);
#else  // Else if this is multiplayer, get player 2's position
  float knobValue = analogRead(player2Pin) / sensorMaxValue;
  player2PosY = clampPaddlePosY(knobValue * LCDHEIGHT);
#endif
}

void updateBall()
{
  ballPosY += ballVelY;
  ballPosX += ballVelX;

  // Top and bottom wall collisions
  if (ballPosY < ballRadius)
  {
    ballPosY = ballRadius;
    ballVelY *= -1.0;
  }
  else if (ballPosY > LCDHEIGHT - ballRadius)
  {
    ballPosY = LCDHEIGHT - ballRadius;
    ballVelY *= -1.0;
  }

  // Left and right wall collisions
  if (ballPosX < ballRadius)
  {
    ballPosX = ballRadius;
    ballVelX = ballSpeedX;
    player2Score++;
  }
  else if (ballPosX > LCDWIDTH - ballRadius)
  {
    ballPosX = LCDWIDTH - ballRadius;
    ballVelX *= -1.0 * ballSpeedX;
    player1Score++;
  }

  // Paddle collisions
  if (ballPosX < player1PosX + ballRadius + halfPaddleWidth)
  {
    if (ballPosY > player1PosY - halfPaddleHeight - ballRadius &&
        ballPosY < player1PosY + halfPaddleHeight + ballRadius)
    {
      ballVelX = ballSpeedX;
      ballVelY = 2.0 * (ballPosY - player1PosY) / halfPaddleHeight;
    }
  }
  else if (ballPosX > player2PosX - ballRadius - halfPaddleWidth)
  {
    if (ballPosY > player2PosY - halfPaddleHeight - ballRadius &&
        ballPosY < player2PosY + halfPaddleHeight + ballRadius)
    {
      ballVelX = -1.0 * ballSpeedX;
      ballVelY = 2.0 * (ballPosY - player2PosY) / halfPaddleHeight;
    }
  }
}

void renderGame()
{
  uView.clear(PAGE);

  renderScores(player1Score, player2Score);
  renderPaddle(player1PosX, player1PosY);
  renderPaddle(player2PosX, player2PosY);
  renderBall(ballPosX, ballPosY);

  uView.display();
  delay(renderDelay);
}

void renderString(int x, int y, String string)
{
  uView.setCursor(x, y);
  uView.print(string);
}

void renderPaddle(int x, int y)
{
  uView.rect(
    x - halfPaddleWidth,
    y - halfPaddleHeight,
    paddleWidth,
    paddleHeight);
}

void renderBall(int x, int y)
{
  uView.circle(x, y, 2);
}

void renderScores(int firstScore, int secondScore)
{
  renderString(10, 0, String(firstScore));
  renderString(LCDWIDTH - 14, 0, String(secondScore));
}

void gameOver(bool didWin)
{
  if (didWin)
  {
#ifdef SINGLE_PLAYER
    renderString(20, 10, "You");
    renderString(20, 30, "Win!");
#else
    renderString(0, 10, "Playr 1");
    renderString(15, 30, "Wins");
#endif
  }
  else
  {
#ifdef SINGLE_PLAYER
    renderString(20, 10, "You");
    renderString(15, 30, "Lose!");
#else
    renderString(0, 10, "Playr 2");
    renderString(15, 30, "Wins");
#endif
  }

  uView.display();
  delay(gameOverDelay);

  // Get ready to start the game again.
  resetGame();
  displayGameStart();
}

