//
// HumanPlayer.cpp for indie in /home/galibe_s/irrlicht/irrlicht-1.8.3/test
//
// Made by stephane galibert
// Login   <galibe_s@epitech.net>
//
// Started on  Fri May  6 17:39:58 2016 stephane galibert
// Last update Tue May 31 14:52:40 2016 stephane galibert
//

#include "HumanPlayer.hpp"
#include "Board.hpp"

size_t bbman::HumanPlayer::NumberOfPlayer = 0;

bbman::HumanPlayer::HumanPlayer(void)
{
  this->_move[Direction::DIR_EAST] =
    std::bind(&bbman::HumanPlayer::moveEast, this, std::placeholders::_1);
  this->_move[Direction::DIR_WEST] =
    std::bind(&bbman::HumanPlayer::moveWest, this, std::placeholders::_1);
  this->_move[Direction::DIR_NORTH] =
    std::bind(&bbman::HumanPlayer::moveNorth, this, std::placeholders::_1);
  this->_move[Direction::DIR_SOUTH] =
    std::bind(&bbman::HumanPlayer::moveSouth, this, std::placeholders::_1);
  this->_inputs[0] =
    std::bind(&bbman::HumanPlayer::inputPlayer1, this, std::placeholders::_1);
  this->_inputs[1] =
    std::bind(&bbman::HumanPlayer::inputPlayer2, this, std::placeholders::_1);
  this->_inits[0] =
    std::bind(&bbman::HumanPlayer::initPlayer1, this, std::placeholders::_1);
  this->_inits[1] =
    std::bind(&bbman::HumanPlayer::initPlayer2, this, std::placeholders::_1);
  this->_mesh = NULL;
  this->_isRunning = false;
  this->_speed = INITIAL_SPEED;
  this->_playerNum = bbman::HumanPlayer::NumberOfPlayer;
  ++bbman::HumanPlayer::NumberOfPlayer;
  this->_prevDirection = Direction::DIR_NONE;
  this->_explosion = NULL;
}

bbman::HumanPlayer *bbman::HumanPlayer::create(void)
{
  if (bbman::HumanPlayer::NumberOfPlayer > 1)
    throw (std::runtime_error("The number of human player can not be greater than 2"));
  return (new HumanPlayer);
}

size_t bbman::HumanPlayer::getAPlayerID(void) const
{
  return (1);
}

bbman::HumanPlayer::~HumanPlayer(void)
{
  if (this->_mesh) {
    this->_mesh->remove();
  }
  for (auto &it : this->_effects) {
    delete (it);
  }
  /*if (this->_explosionTask && this->_explosionTask->isRunning()) {
    this->_explosionTask->stop();
    while (!this->_explosionTask->isFinished());
    delete (this->_explosionTask);
    }*/
  if (this->_explosion) {
    delete (this->_explosion);
  }
  --bbman::HumanPlayer::NumberOfPlayer;
}

void bbman::HumanPlayer::init(bbman::Irrlicht &irr)
{
  try {
    if (this->_inits.find(this->_playerNum) != std::end(this->_inits)) {
      this->_inits[this->_playerNum](irr);
    }
    //this->_explosionTask = new ExplosionTask(irr);
    //this->_explosion.init(irr);
    this->_explosion = new Explosion;
    this->_explosion->init(irr);
    addBomb(new ExplodingBomb(this));
    this->_alive = true;
  } catch (std::runtime_error const& e) {
    throw (e);
  }
}

void bbman::HumanPlayer::update(bbman::Irrlicht &irr, irr::f32 delta)
{
  (void)irr;
  if (this->_alive) {
    move(delta);
    updateEffets(delta);
  }
  /*if (this->_explosionTask && this->_explosionTask->isRunning()
      && this->_explosionTask->isFinished()) {
    delete (this->_explosionTask);
    this->_explosionTask = NULL;
    }*/
  if (this->_explosion) {
    this->_explosion->update(delta);
    if (this->_explosion->hasFinished()) {
      delete (this->_explosion);
      this->_explosion = NULL;
    }
  }
}

void bbman::HumanPlayer::play(bbman::Irrlicht &irr, bbman::Board *board)
{
  if (this->_alive) {
    checkDirection(board);
    if (this->_action == bbman::ACT_BOMB) {
      dropBomb(irr, board);
    }
  }
}

void bbman::HumanPlayer::checkDirection(bbman::Board *board)
{
  if (this->_alive) {
    if (!board->isInNode(getPosition())) {
      if ((this->_prevDirection == Direction::DIR_EAST
	   || this->_prevDirection == Direction::DIR_WEST)
	  &&
	  (this->_direction == Direction::DIR_NORTH
	 || this->_direction == Direction::DIR_SOUTH)) {
	this->_direction = Direction::DIR_NONE;
      }
      else if ((this->_prevDirection == Direction::DIR_NORTH
		|| this->_prevDirection == Direction::DIR_SOUTH)
	       &&
	       (this->_direction == Direction::DIR_EAST
		|| this->_direction == Direction::DIR_WEST)) {
	this->_direction = Direction::DIR_NONE;
      }
    }
    else if (!board->isValidMove(getPosition(), this->_direction)) {
      this->_direction = Direction::DIR_NONE;
    }
  }
}

void bbman::HumanPlayer::dropBomb(bbman::Irrlicht &irr, bbman::Board *board)
{
  IBomb *newBomb = createBomb(irr);
  if (newBomb) {
    irr::core::vector3df pos = getPosition();

    pos.X = board->getScale().X / 2 + std::floor(pos.X)
      - (int)(std::floor(pos.X)) % (int)board->getScale().X;
    pos.Z = board->getScale().Z / 2 + std::floor(pos.Z)
      - (int)(std::floor(pos.Z)) % (int)board->getScale().Z;
    newBomb->setPosition(pos);
    if (!board->addBomb(newBomb)) {
      addBomb(newBomb);
    }
  }
}

void bbman::HumanPlayer::addBomb(bbman::IBomb *bomb)
{
  this->_bombManager.addBomb(bomb);
}

void bbman::HumanPlayer::setPosition(irr::core::vector3df const& pos)
{
  irr::f32 x = (int)(pos.X / 10.f) * 10.f;
  irr::f32 y = (int)(pos.Z / 10.f) * 10.f;

  x += (10.f / 2.f);
  y += (10.f / 2.f);
  this->_mesh->setPosition(irr::core::vector3df(x, 0.f, y));
  this->_mesh->updateAbsolutePosition();
}

irr::core::vector3df const& bbman::HumanPlayer::getPosition(void) const
{
  return (this->_mesh->getPosition());
}

irr::core::aabbox3df const bbman::HumanPlayer::getBoundingBox(void) const
{
  if (this->_alive) {
    return (this->_mesh->getTransformedBoundingBox());
  }
  return (irr::core::aabbox3df());
}

bool bbman::HumanPlayer::isColliding(irr::core::aabbox3df const& box) const
{
  if (this->_alive) {
    return (box.intersectsWithBox(this->getBoundingBox()));
  }
  return (false);
}

void bbman::HumanPlayer::explode(Board *board)
{
  (void)board;
  if (this->_alive) {
    this->_alive = false;
    /*this->_mesh->remove();
      this->_mesh = NULL;*/
    this->_mesh->setVisible(false);
    std::cerr << "player " + std::to_string(this->_playerNum) + " died" << std::endl;
  }
}

void bbman::HumanPlayer::playExplosion(void)
{
  /*if (this->_explosionTask && this->_explosionTask->isFinished()) {
    this->_explosionTask->setPosition(getPosition());
    this->_explosionTask->setVisible(true);
    tools::StaticTools::ThreadPool->addTask(this->_explosionTask);
    }*/
  if (this->_explosion) {
    this->_explosion->play(getPosition());
  }
}

bool bbman::HumanPlayer::isRunning(void) const
{
  return (this->_isRunning);
}

bool bbman::HumanPlayer::hasExplosed(void) const
{
  return (!this->_alive);
}

irr::core::vector3d<irr::s32> const& bbman::HumanPlayer::getPosInMap(irr::core::vector3df const& scale)
{
  this->_posInMap.X = getPosition().X / scale.X;
  this->_posInMap.Z = getPosition().Z / scale.Z;
  return (this->_posInMap);
}

bbman::BombManager const& bbman::HumanPlayer::getBombManager(void) const
{
  return (this->_bombManager);
}

std::list<bbman::IEffect *> const& bbman::HumanPlayer::getEffects(void) const
{
  return (this->_effects);
}

void bbman::HumanPlayer::setRotation(irr::s32 rotation)
{
  if (this->_mesh) {
    this->_mesh->setRotation(irr::core::vector3df(0, rotation, 0));
  }
}

irr::s32 bbman::HumanPlayer::getRotation(void) const
{
  return ((irr::s32)this->_mesh->getRotation().Y);
}

void bbman::HumanPlayer::setAlive(bool v)
{
  this->_alive = v;
  if (!this->_alive && this->_mesh) {
    this->_mesh->setVisible(false);
  }
}

bool bbman::HumanPlayer::isAlive(void) const
{
  return (this->_alive);
}

bool bbman::HumanPlayer::input(bbman::InputListener &inputListener)
{
  this->_direction = Direction::DIR_NONE;
  this->_action = Action::ACT_NONE;
  if (this->_inputs.find(this->_playerNum) != this->_inputs.cend()) {
    this->_inputs.at(this->_playerNum)(inputListener);
  }
  return (false);
}

size_t bbman::HumanPlayer::getSpeed(void) const
{
  return (this->_speed);
}

void bbman::HumanPlayer::setSpeed(size_t speed)
{
  this->_speed = speed;
}

void bbman::HumanPlayer::addEffect(IEffect *effect)
{
  if (this->_alive) {
    /*if (std::find_if(std::begin(this->_effects), std::end(this->_effects),
		     [&effect](IEffect *buff) {
		       if (effect->getEffectID() == buff->getEffectID())
			 buff->restart();
		       return (true);
		       return (false);
		   }) == std::end(this->_effects)) {
		   effect->enable();*/
    effect->enable();
    this->_effects.push_back(effect);
      /*}
    else {
      delete (effect);
      }*/
  }
}

void bbman::HumanPlayer::updateEffets(irr::f32 delta)
{
  for (std::list<IEffect *>::iterator it = std::begin(this->_effects);
       it != std::end(this->_effects); ) {
    if ((*it)->isFinished()) {
      delete (*it);
      it = this->_effects.erase(it);
    }
    else {
      (*it)->update(delta);
      ++it;
    }
  }
}

void bbman::HumanPlayer::move(irr::f32 delta)
{
  if (this->_direction == Direction::DIR_NONE) {
    this->_mesh->setCurrentFrame(5);
    this->_mesh->setAnimationSpeed(0);
    this->_isRunning = false;
  }
  else {
    if (!this->_isRunning) {
      this->_mesh->setAnimationSpeed(15);
      this->_mesh->setFrameLoop(0, 13);
      this->_isRunning = true;
    }
    this->_move.at(this->_direction)(delta);
  }
}

void bbman::HumanPlayer::moveEast(irr::f32 delta)
{
  this->_prevDirection = Direction::DIR_EAST;
  irr::core::vector3df playerPos = this->_mesh->getPosition();
  playerPos.X += this->_speed * delta;
  this->_mesh->setPosition(playerPos);
  this->_mesh->setRotation(irr::core::vector3df(0, -90, 0));
}

void bbman::HumanPlayer::moveWest(irr::f32 delta)
{
  this->_prevDirection = Direction::DIR_WEST;
  irr::core::vector3df playerPos = this->_mesh->getPosition();
  playerPos.X -= this->_speed * delta;
  this->_mesh->setPosition(playerPos);
  this->_mesh->setRotation(irr::core::vector3df(0, 90, 0));
}

void bbman::HumanPlayer::moveNorth(irr::f32 delta)
{
  this->_prevDirection = Direction::DIR_NORTH;
  irr::core::vector3df playerPos = this->_mesh->getPosition();
  playerPos.Z += this->_speed * delta;
  this->_mesh->setPosition(playerPos);
  this->_mesh->setRotation(irr::core::vector3df(0, 180, 0));
}

void bbman::HumanPlayer::moveSouth(irr::f32 delta)
{
  this->_prevDirection = Direction::DIR_SOUTH;
  irr::core::vector3df playerPos = this->_mesh->getPosition();
  playerPos.Z -= this->_speed * delta;
  this->_mesh->setPosition(playerPos);
  this->_mesh->setRotation(irr::core::vector3df(0, 0, 0));
}

void bbman::HumanPlayer::inputPlayer1(bbman::InputListener &listener)
{
  if (this->_alive) {
    irr::SEvent::SJoystickEvent const& joy = listener.getJoystickState(0);
    irr::f32 moveH = joy.Axis[irr::SEvent::SJoystickEvent::AXIS_X] / 32767.f;
    irr::f32 moveV = joy.Axis[irr::SEvent::SJoystickEvent::AXIS_Y] / 32767.f;
    if (std::fabs(moveH) < 0.50f) {
      moveH = 0;
    }
    if (std::fabs(moveV) < 0.50f) {
      moveV = 0;
    }

    if(listener.IsKeyDown(irr::KEY_KEY_Z) || moveV < 0) {
      this->_direction = Direction::DIR_NORTH;
    }
    else if(listener.IsKeyDown(irr::KEY_KEY_S) || moveV > 0) {
      this->_direction = Direction::DIR_SOUTH;
    }
    else if(listener.IsKeyDown(irr::KEY_KEY_Q) || moveH < 0) {
      this->_direction = Direction::DIR_WEST;
    }
    else if(listener.IsKeyDown(irr::KEY_KEY_D) || moveH > 0) {
      this->_direction = Direction::DIR_EAST;
    }

    if (listener.IsKeyDown(irr::KEY_SPACE) || joy.IsButtonPressed(1)) {
      this->_action |= Action::ACT_BOMB;
    }
  }
}

void bbman::HumanPlayer::inputPlayer2(bbman::InputListener &listener)
{
  if (this->_alive) {
    irr::SEvent::SJoystickEvent const& joy = listener.getJoystickState(1);
    irr::f32 moveH = joy.Axis[irr::SEvent::SJoystickEvent::AXIS_X] / 32767.f;
    irr::f32 moveV = joy.Axis[irr::SEvent::SJoystickEvent::AXIS_Y] / 32767.f;
    if (std::fabs(moveH) < 0.50f) {
      moveH = 0.f;
    }
    if (std::fabs(moveV) < 0.50f) {
      moveV = 0.f;
    }

    if(listener.IsKeyDown(irr::KEY_UP) || moveV < 0.f) {
      this->_direction = Direction::DIR_NORTH;
    }
    else if(listener.IsKeyDown(irr::KEY_DOWN) || moveV > 0.f) {
      this->_direction = Direction::DIR_SOUTH;
    }
    else if(listener.IsKeyDown(irr::KEY_LEFT) || moveH < 0.f) {
      this->_direction = Direction::DIR_WEST;
    }
    else if(listener.IsKeyDown(irr::KEY_RIGHT) || moveH > 0.f) {
      this->_direction = Direction::DIR_EAST;
    }

    if (listener.IsKeyDown(irr::KEY_RETURN) || joy.IsButtonPressed(1)) {
      this->_action |= Action::ACT_BOMB;
    }
  }
}

void bbman::HumanPlayer::initPlayer1(bbman::Irrlicht &irr)
{
  std::string txt = "./asset/media/ninja.b3d";
  this->_mesh = irr.getSmgr()->addAnimatedMeshSceneNode(irr.getMesh(txt.data()));
  if (this->_mesh) {
    this->_mesh->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    this->_mesh->setAnimationSpeed(0);
    this->_mesh->setScale(irr::core::vector3df(1.5f, 2.f, 1.5f));
    this->_mesh->setRotation(irr::core::vector3df(0, 180, 0));
  }
  else {
    throw (std::runtime_error("can not create player " + std::to_string(this->_playerNum)));
  }
}

void bbman::HumanPlayer::initPlayer2(Irrlicht &irr)
{
  std::string txt = "./asset/media/ninja.b3d";
  this->_mesh = irr.getSmgr()->addAnimatedMeshSceneNode(irr.getMesh(txt.data()));
  if (this->_mesh) {
    this->_mesh->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    this->_mesh->setAnimationSpeed(0);
    this->_mesh->setScale(irr::core::vector3df(1.5f, 2.f, 1.5f));
    this->_mesh->setRotation(irr::core::vector3df(0, 180, 0));
  }
  else {
    throw (std::runtime_error("can not create player " + std::to_string(this->_playerNum)));
  }
}

size_t bbman::HumanPlayer::getPlayerNumber(void) const
{
  return (this->_playerNum);
}

bbman::IBomb *bbman::HumanPlayer::createBomb(bbman::Irrlicht &irr)
{
  IBomb *bomb = this->_bombManager.getSelectedBomb();
  if (bomb) {
    bomb->init(irr);
  }
  return (bomb);
}
