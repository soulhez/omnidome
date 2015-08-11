#include <omni/proj/Tuning.h>

#include <omni/util.h>
#include <omni/proj/Setup.h>
#include <omni/proj/ScreenSetup.h>
#include <QColor>


namespace omni
{
  namespace proj
  {
    Tuning::Tuning() :
      color_("#FFFFFF"),
      blendMask_(*this)
    {
    }

    Tuning::Tuning(const QColor& _color) :
      color_(_color),
      blendMask_(*this)
    {
    }

    void Tuning::setScreen(QScreen const* _screen, int _subScreenIndex)
    {
      projector_.setScreen(_screen,_subScreenIndex);
    }

    QScreen const* Tuning::screen() const
    {
      return projector_.screen();
    }

    int Tuning::subScreenIndex() const
    {
      return projector_.subScreenIndex();
    }
    
    Projector& Tuning::projector()
    {
      return projector_;
    }

    Projector const& Tuning::projector() const
    {
      return projector_;
    }

    WarpGrid& Tuning::warpGrid()
    {
      return warpGrid_;
    }

    WarpGrid const& Tuning::warpGrid() const
    {
      return warpGrid_;
    }

    BlendMask& Tuning::blendMask()
    {
      return blendMask_;
    }

    BlendMask const& Tuning::blendMask() const
    {
      return blendMask_;
    }

    QColor Tuning::color() const
    {
      return color_;
    }

    void Tuning::setColor(QColor const& _color)
    {
      color_=_color;
    }

    bool Tuning::hasScreen() const
    {
      return projector_.screen() != nullptr;
    }

    bool Tuning::resolutionToBeChanged(proj::Screen const* _screen) const
    {
      return false;
    }

    int Tuning::width() const
    {
      return !projector_.screen() ? 
        ScreenSetup::standardScreen()->size().width() : 
        ScreenSetup::subScreenWidth(projector_.screen());
    }

    int Tuning::height() const
    {
      return !projector_.screen() ? 
        ScreenSetup::standardScreen()->size().height() : 
        projector_.screen()->size().height();
    }
 
    bool operator==(Tuning const& _lhs, Tuning const& _rhs)
    {
      return 
        OMNI_TEST_MEMBER_EQUAL(color_) &&
        OMNI_TEST_MEMBER_EQUAL(projector_) &&
        OMNI_TEST_PTR_MEMBER_EQUAL(projectorSetup_) &&
        OMNI_TEST_MEMBER_EQUAL(warpGrid_) &&
        OMNI_TEST_MEMBER_EQUAL(blendMask_);
    }
  }
}

QDataStream& operator>>(QDataStream& _stream, omni::proj::Tuning& _tuning)
{
  using namespace omni::util;
  
  QColor _color;
  _stream >> _color;
  _tuning.setColor(_color);
  _stream >> _tuning.projector();
  _stream >> _tuning.warpGrid();
  _stream >> _tuning.blendMask();

  return _stream;
}

QDataStream& operator<<(QDataStream& _stream, omni::proj::Tuning const& _tuning)
{
  using namespace omni::util;
  
  _stream << _tuning.color();
  _stream << _tuning.projector();
  _stream << _tuning.warpGrid();
  _stream << _tuning.blendMask();
  return _stream;
}


