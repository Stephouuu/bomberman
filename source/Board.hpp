//
// Board.hpp for indie in /home/galibe_s/irrlicht/irrlicht-1.8.3/test
//
// Made by stephane galibert
// Login   <galibe_s@epitech.net>
//
// Started on  Thu May  5 02:14:22 2016 stephane galibert
// Last update Tue May 10 05:12:38 2016 stephane galibert
//

#ifndef _BOARD_HPP_
# define _BOARD_HPP_

# include <iostream>
# include <irrlicht.h>
# include <stdexcept>
# include <vector>

# include "IEntity.hpp"
# include "Map.hpp"
# include "ItemID.hpp"
# include "IBlock.hpp"
# include "IndestructibleBlock.hpp"

namespace bbman
{
  class Board : public IEntity
  {
  public:
    Board(void);
    virtual ~Board(void);
    virtual void init(Irrlicht &irr);
    virtual void update(Irrlicht &irr, irr::f32 delta);
    virtual void setPosition(irr::core::vector3df const& pos);
    virtual irr::core::vector3df const& getPosition(void) const;
    virtual irr::core::aabbox3df const getBoundingBox(void) const;
    virtual bool isColliding(irr::core::aabbox3df const& box) const;
    bool isOutside(irr::core::vector3df const& pos);
    irr::core::vector3df const& getSize(void) const;
    irr::core::vector3df const& getScale(void) const;
    Map<ItemID> const& getMap(void) const;
  private:
    void initTerrain(Irrlicht &irr);
    void initMap(void);
    void initMesh(Irrlicht &irr);
    irr::scene::IMeshSceneNode *_backgroundMesh;
    irr::video::SMaterial _texture;
    std::vector<IBlock *> _blocks;
    Map<ItemID> _map;
    irr::core::vector3df _size;
    irr::core::vector3df _scale;
  };
}

#endif /* !_BOARD_HPP_ */