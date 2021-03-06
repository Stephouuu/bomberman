//
// HumanPlayer.cpp for indie in /home/galibe_s/irrlicht/irrlicht-1.8.3/test
//
// Made by stephane galibert
// Login   <galibe_s@epitech.net>
//
// Started on  Fri May  6 17:39:58 2016 stephane galibert
// Last update Sun Jun  5 16:23:45 2016 stephane galibert
//

#include "HumanPlayer.hpp"
#include "Board.hpp"

size_t bbman::HumanPlayer::NumberOfPlayer = 0;
bbman::CacheManager<std::string, bbman::MemoryFile> bbman::HumanPlayer::SoundCache;

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
  this->_devices[0] =
    std::bind(&bbman::HumanPlayer::deviceKeyboard1, this, std::placeholders::_1);
  this->_devices[1] =
    std::bind(&bbman::HumanPlayer::deviceKeyboard2, this, std::placeholders::_1);
  this->_devices[2] =
    std::bind(&bbman::HumanPlayer::deviceJoystick1, this, std::placeholders::_1);
  this->_devices[3] =
    std::bind(&bbman::HumanPlayer::deviceJoystick2, this, std::placeholders::_1);
  this->_anim = true;
  this->_mesh = NULL;
  this->_isRunning = false;
  this->_speed = INITIAL_SPEED;
  this->_playerNum = bbman::HumanPlayer::NumberOfPlayer;
  ++bbman::HumanPlayer::NumberOfPlayer;
  this->_prevDirection = Direction::DIR_NONE;
  this->_explosion = NULL;
  this->_deviceID = 0;
  this->_final = true;
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
  if (this->_explosion) {
    delete (this->_explosion);
  }
  --bbman::HumanPlayer::NumberOfPlayer;
}

void bbman::HumanPlayer::init(bbman::Irrlicht &irr, std::string const& color)
{
  (void)irr;
  (void)color;
}

void bbman::HumanPlayer::init(bbman::Irrlicht &irr, int deviceID,
			      std::string const& color)
{
  IBomb *bomb = NULL;
  try {
    this->_color = color;
    this->_deviceID = deviceID;
    initPlayer(irr);
    this->_explosion = new Explosion;
    this->_explosion->init(irr, color);
    bomb = new ExplodingBomb(this);
    bomb->setColor(color);
    addBomb(bomb);
    this->_alive = true;

    try {
      if (!SoundCache.find("death")) {
	SoundCache.insert("death", MemoryFile("./asset/sound/death.wav"));
	SoundCache["death"].load();
      }
      this->_sounds.addSample("death", SoundCache["death"]);
      this->_sounds.setVolumeBySample("death", tools::StaticTools::volume("effect"));
    } catch (std::runtime_error const& e) {
      std::cerr << e.what() << std::endl;
    }
  } catch (std::runtime_error const& e) {
    throw (e);
  }
}

void bbman::HumanPlayer::update(bbman::Irrlicht &irr, irr::f32 delta)
{
  (void)irr;
  if (this->_alive) {
    this->_delta = delta;
    updateEffets(delta);
  }
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
    if (!board->isInNode(this->getPosition())) {
      this->_direction = this->_prevDirection;
    }
    if (this->_action == bbman::ACT_BOMB) {
      dropBomb(irr, board);
    }
    move(this->_delta);
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

    pos.X = board->getScale().X / 2 + std::floor(pos.X) - (int)(std::floor(pos.X)) % (int)board->getScale().X;
    pos.Z = board->getScale().Z / 2 + std::floor(pos.Z) - (int)(std::floor(pos.Z)) % (int)board->getScale().Z;
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
    this->_mesh->setVisible(false);
    try {
      this->_sounds.play("death");
    } catch (std::runtime_error const& e) {
      std::cerr << e.what() << std::endl;
    }
  }
}

void bbman::HumanPlayer::playExplosion(void)
{
  if (this->_anim && this->_explosion) {
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

std::string const& bbman::HumanPlayer::getColor(void) const
{
  return (this->_color);
}

void bbman::HumanPlayer::setColor(std::string const& color)
{
  this->_color = color;
}

bool bbman::HumanPlayer::isAlive(void) const
{
  return (this->_alive);
}

bool bbman::HumanPlayer::input(bbman::InputListener &inputListener)
{
  this->_direction = Direction::DIR_NONE;
  this->_action = Action::ACT_NONE;
  if (this->_devices.find(this->_deviceID) != this->_devices.cend()
      && this->_alive) {
    this->_devices.at(this->_deviceID)(inputListener);
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
    effect->enable();
    this->_effects.push_back(effect);
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
    this->_mesh->setFrameLoop(1, 51);
    this->_mesh->setAnimationSpeed(15);
    this->_isRunning = false;
  }
  else {
    if (!this->_isRunning) {
      this->_mesh->setAnimationSpeed(25);
      this->_mesh->setFrameLoop(60, 105);
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
  this->_mesh->setRotation(irr::core::vector3df(0, 90, 0));
}

void bbman::HumanPlayer::moveWest(irr::f32 delta)
{
  this->_prevDirection = Direction::DIR_WEST;
  irr::core::vector3df playerPos = this->_mesh->getPosition();
  playerPos.X -= this->_speed * delta;
  this->_mesh->setPosition(playerPos);
  this->_mesh->setRotation(irr::core::vector3df(0, -90, 0));
}

void bbman::HumanPlayer::moveNorth(irr::f32 delta)
{
  this->_prevDirection = Direction::DIR_NORTH;
  irr::core::vector3df playerPos = this->_mesh->getPosition();
  playerPos.Z += this->_speed * delta;
  this->_mesh->setPosition(playerPos);
  this->_mesh->setRotation(irr::core::vector3df(0, 0, 0));
}

void bbman::HumanPlayer::moveSouth(irr::f32 delta)
{
  this->_prevDirection = Direction::DIR_SOUTH;
  irr::core::vector3df playerPos = this->_mesh->getPosition();
  playerPos.Z -= this->_speed * delta;
  this->_mesh->setPosition(playerPos);
  this->_mesh->setRotation(irr::core::vector3df(0, 180, 0));
}

void bbman::HumanPlayer::deviceKeyboard1(bbman::InputListener &listener)
{
  if(listener.IsKeyDown(irr::KEY_KEY_Z)) {
    this->_direction = Direction::DIR_NORTH;
  }
  else if(listener.IsKeyDown(irr::KEY_KEY_S)) {
    this->_direction = Direction::DIR_SOUTH;
  }
  else if(listener.IsKeyDown(irr::KEY_KEY_Q)) {
    this->_direction = Direction::DIR_WEST;
  }
  else if(listener.IsKeyDown(irr::KEY_KEY_D)) {
    this->_direction = Direction::DIR_EAST;
  }

  if (listener.IsKeyDown(irr::KEY_SPACE)) {
    this->_action |= Action::ACT_BOMB;
  }
}

void bbman::HumanPlayer::deviceKeyboard2(bbman::InputListener &listener)
{
  if(listener.IsKeyDown(irr::KEY_UP)) {
    this->_direction = Direction::DIR_NORTH;
  }
  else if(listener.IsKeyDown(irr::KEY_DOWN)) {
    this->_direction = Direction::DIR_SOUTH;
  }
  else if(listener.IsKeyDown(irr::KEY_LEFT)) {
    this->_direction = Direction::DIR_WEST;
  }
  else if(listener.IsKeyDown(irr::KEY_RIGHT)) {
    this->_direction = Direction::DIR_EAST;
  }

  if (listener.IsKeyDown(irr::KEY_RETURN)) {
    this->_action |= Action::ACT_BOMB;
  }
}

void bbman::HumanPlayer::deviceJoystick1(bbman::InputListener &listener)
{
  irr::SEvent::SJoystickEvent const& joy = listener.getJoystickState(0);
  irr::f32 moveH = joy.Axis[irr::SEvent::SJoystickEvent::AXIS_X] / 32767.f;
  irr::f32 moveV = joy.Axis[irr::SEvent::SJoystickEvent::AXIS_Y] / 32767.f;
  if (std::fabs(moveH) < 0.50f) {
    moveH = 0;
  }
  if (std::fabs(moveV) < 0.50f) {
    moveV = 0;
  }
  if(moveV < 0) {
    this->_direction = Direction::DIR_NORTH;
  }
  else if(moveV > 0) {
    this->_direction = Direction::DIR_SOUTH;
  }
  else if(moveH < 0) {
    this->_direction = Direction::DIR_WEST;
  }
  else if(moveH > 0) {
    this->_direction = Direction::DIR_EAST;
  }

  if (joy.IsButtonPressed(1)) {
    this->_action |= Action::ACT_BOMB;
  }
}

void bbman::HumanPlayer::deviceJoystick2(bbman::InputListener &listener)
{
  irr::SEvent::SJoystickEvent const& joy = listener.getJoystickState(1);
  irr::f32 moveH = joy.Axis[irr::SEvent::SJoystickEvent::AXIS_X] / 32767.f;
  irr::f32 moveV = joy.Axis[irr::SEvent::SJoystickEvent::AXIS_Y] / 32767.f;
  if (std::fabs(moveH) < 0.50f) {
    moveH = 0.f;
  }
  if (std::fabs(moveV) < 0.50f) {
    moveV = 0.f;
  }

  if(moveV < 0.f) {
    this->_direction = Direction::DIR_NORTH;
  }
  else if (moveV > 0.f) {
    this->_direction = Direction::DIR_SOUTH;
  }
  else if (moveH < 0.f) {
    this->_direction = Direction::DIR_WEST;
  }
  else if (moveH > 0.f) {
    this->_direction = Direction::DIR_EAST;
  }

  if (joy.IsButtonPressed(1)) {
    this->_action |= Action::ACT_BOMB;
  }
}

void bbman::HumanPlayer::initPlayer(bbman::Irrlicht &irr)
{
  std::string fbx = "./asset/perso/perso.fbx";
  std::string diffuse = "./asset/perso/texture/diffuse/" + this->_color + ".png";
  std::string iri = "asset/perso/texture/illum/" + this->_color + ".png";
  std::string normal = "./asset/perso/texture/normal/normal.png";

  this->_mesh = irr.getSmgr()->addAnimatedMeshSceneNode(irr.getMesh(fbx.data()));
  if (this->_mesh) {
    this->_mesh->setMaterialTexture(0, irr.getTexture(diffuse.data()));
    this->_mesh->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    this->_mesh->setAnimationSpeed(0);
    this->_mesh->setScale(irr::core::vector3df(0.8f, 0.8f, 0.8f));
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
    bomb->addRange(this->_power);
    bomb->init(irr, this->_color);
  }
  return (bomb);
}

void bbman::HumanPlayer::setDeviceID(int id)
{
  this->_deviceID = id;
}

int bbman::HumanPlayer::getDeviceID(void) const
{
  return (this->_deviceID);
}

void bbman::HumanPlayer::disableAnimation(void)
{
  this->_anim = false;
}

int bbman::HumanPlayer::getScoreValue(void) const
{
  return (10);
}

std::string const& bbman::HumanPlayer::getDifficulty(void) const
{
  return (this->_difficutly);
}

void bbman::HumanPlayer::finalPosition(void)
{
  if (this->_alive && this->_final) {
    this->_mesh->setRotation(irr::core::vector3df(0, 180, 0));
    this->_mesh->setFrameLoop(1, 51);
    this->_mesh->setAnimationSpeed(30);
    this->_isRunning = false;
    this->_final = false;
  }
}
