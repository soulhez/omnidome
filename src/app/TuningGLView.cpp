#include <omni/ui/TuningGLView.h>

#include <QDebug>
#include <QResizeEvent>
#include <QMatrix4x4>
#include <omni/visual/util.h>
#include <omni/visual/Rectangle.h>

namespace omni
{
  namespace ui
  {
    TuningGLView::TuningGLView(QWidget* _parent) :
      GLView(_parent),
      cursorPosition_(0.0,0.0)
    {
    }

    TuningGLView::~TuningGLView()
    {
    }

    omni::proj::Tuning* TuningGLView::tuning()
    {
      return this->session_->session().tunings()[index_];
    }

    omni::proj::Tuning const* TuningGLView::tuning() const
    {
      return this->session_->session().tunings()[index_];
    }

    void TuningGLView::setTuningIndex(int _index)
    {
      index_=_index;
      auto* _tuning = tuning();

      if (!_tuning) return;
      tuning_.reset(new visual::Tuning(*_tuning));
    }

    bool TuningGLView::keepAspectRatio() const
    {
      return keepAspectRatio_;
    }

    void TuningGLView::setKeepAspectRatio(bool _keepAspectRatio)
    {
      keepAspectRatio_ = _keepAspectRatio;
      sizePolicy().setHeightForWidth(keepAspectRatio_);
      update();
    }

    bool TuningGLView::viewOnly() const
    {
      return viewOnly_;
    }

    void TuningGLView::setViewOnly(bool _viewOnly)
    {
      viewOnly_ = _viewOnly;
      update();
    }

    bool TuningGLView::isDrawingEnabled() const
    {
      return drawingEnabled_;
    }

    void TuningGLView::setDrawingEnabled(bool _drawingEnabled)
    {
      drawingEnabled_ = _drawingEnabled;
      update();
    }

    bool TuningGLView::isFullscreen() const
    {
      return fullscreen_;
    }

    float TuningGLView::border() const
    {
      return border_;
    }

    void TuningGLView::setBorder(float _border)
    {
      border_ = _border;
      update();
    }

    /// Show different content for different session modes
    void TuningGLView::sessionModeChange()
    {
    }

    void TuningGLView::setFullscreen(Screen const& _screen)
    {
      setParent(nullptr);
      setWindowFlags( Qt::CustomizeWindowHint );
      setWindowFlags(Qt::FramelessWindowHint);
      raise();
      setGeometry(_screen.rect());
      fullscreen_ = true;
    }

    void TuningGLView::mouseMoveEvent(QMouseEvent *event)
    {
      if (!session() || (viewOnly() && !showCursor_)) return;

      auto&& _rect = viewRect();
      float dx = float(event->x() - mousePosition().x()) / width() * _rect.width();
      float dy = float(event->y() - mousePosition().y()) / height() * _rect.height();

      auto _mode = session()->mode();

      if (mouseDown_)
      {
        auto& _warpGrid = tuning()->warpGrid();

        // Manipulate selected warp grid point, when mouse button is pressed
        if (_mode == Session::Mode::WARP)
        {
          auto&& _selectedPoints = _warpGrid.getSelected();
          for (auto& _selected : _selectedPoints)
          {
            _selected->pos() += QPointF(dx,-dy);
          }
        }
        else if (_mode == Session::Mode::BLEND)
        {
          leftOverDistance_ = tuning()->blendMask().drawLine(mousePosition_,event->pos(),leftOverDistance_);
        }
      }
      mousePosition_ = event->pos();
      /*if (!viewOnly())
      {
        //cursorPosition() = screenPos();
      }
      tuning_.drawCursor();*/

      update();
    }

    void TuningGLView::mousePressEvent(QMouseEvent *event)
    {
      QOpenGLWidget::mousePressEvent(event);

      if (!session() || viewOnly()) return;

      this->mousePosition_ = event->pos();
      auto&& _newPos = screenPos(this->mousePosition_);
      mouseDown_ = true;

      auto _mode = session()->mode();
      if (_mode == Session::Mode::WARP)
      {
        auto& _warpGrid = tuning()->warpGrid();

        auto&& _selectedPoints = _warpGrid.getSelected();
        auto _p = _warpGrid.selectNearest(QPointF(_newPos.x(),_newPos.y()));
        if (!_p) return;

        // Deselect points if ctrl key is not pressed
        if (!(event->modifiers() & Qt::ControlModifier))
        {
          _warpGrid.selectNone();
        }
        // Select point if it is not selected already or
        // number of selected point is larger than 1
        _p->setSelected(!_p->selected() || (_selectedPoints.size() > 1));
      }
      else if (_mode == Session::Mode::BLEND)
      {
        bool _inv = tuning()->blendMask().brush().invert();

        // Invert brush on right click
        if (event->button() == Qt::RightButton)
        {
          tuning()->blendMask().brush().setInvert(!_inv);
        }
        leftOverDistance_ = 0.0;
      }

      update();
    }

    void TuningGLView::mouseReleaseEvent(QMouseEvent *event)
    {
      if (!session() || viewOnly()) return;

      mouseDown_ = false;
      makeCurrent();
      auto _mode = session()->mode();
      if (_mode == Session::Mode::BLEND)
      {
        leftOverDistance_ = 0.0;

        // Invert brush on right click
        bool _inv = tuning()->blendMask().brush().invert();
        if (event->button() == Qt::RightButton)
        {
          tuning()->blendMask().brush().setInvert(!_inv);
        }
      }
      update();
    }

    void TuningGLView::wheelEvent(QWheelEvent* event)
    {
    }

    void TuningGLView::keyPressEvent(QKeyEvent* event)
    {
    }

    bool TuningGLView::initialize()
    {
      if (context())
      {
        frameBuffer_.reset(new QOpenGLFramebufferObject(512,512));
      }

      return context() != nullptr;
    }

    QRectF TuningGLView::viewRect() const
    {
      float _projAspect = float(tuning()->width()) / tuning()->height();
      float _viewAspect = float(width()) / height();
      float b = border_ * 0.5;
      float _left = -0.5 - b,_right = 0.5 + b,_bottom = -0.5 - b,_top = 0.5 + b;

      if (keepAspectRatio())
      {
        if (_projAspect > _viewAspect)
        {
          _top *= _projAspect / _viewAspect;
          _bottom *=  _projAspect / _viewAspect;
        }
        else
        {
          _left *= _viewAspect / _projAspect;
          _right *= _viewAspect / _projAspect;
        }
      }
      return QRectF(QPointF(_left,_top),QPointF(_right,_bottom));
    }

    QPointF TuningGLView::screenPos(QPointF const& _pos) const
    {
      QRectF&& _rect = viewRect();
      QPointF _p = QPointF(_pos.x() / width() - 0.5,_pos.y() / height() - 0.5);
      return QPointF(_p.x() * _rect.width(),-_p.y() * _rect.height());
    }

    void TuningGLView::drawCanvas()
    {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glMultMatrixf(tuning()->projector().projectionMatrix().constData());
      glMatrixMode(GL_MODELVIEW);

      glLoadIdentity();

      glDisable(GL_LIGHTING);
      session_->drawCanvas();
    }
      
    /// Update warp buffer which contains image of projector perspective
    void TuningGLView::updateWarpBuffer()
    {
      // If tuning size has changed, reset warpGrid framebuffer
      if (warpGridBuffer_)
      {
        if ((warpGridBuffer_->width() != tuning()->width()) ||
            (warpGridBuffer_->height() != tuning()->height()))
          warpGridBuffer_.reset();
      }

      // If warp grid framebuffer is empty, make a new one
      if (!warpGridBuffer_)
      {
        warpGridBuffer_.reset(new QOpenGLFramebufferObject(
                                tuning()->width(),
                                tuning()->height()));
      }

      // Draw projector's perspective on framebuffer texture
      visual::with_current_context([this](QOpenGLFunctions& _)
      {
        _.glEnable(GL_TEXTURE_2D);
        _.glDisable(GL_LIGHTING);
        visual::draw_on_framebuffer(warpGridBuffer_,
                                    [&](QOpenGLFunctions& _) // Projection Operation
        {
          glMultMatrixf(tuning()->projector().projectionMatrix().constData());
        },
        [&](QOpenGLFunctions& _) // Model View Operation
        {
          _.glClearColor(0.0,0.0,0.0,1.0);
          session_->drawCanvas();
        });
      });
    }
      
    template<typename F> 
    void TuningGLView::drawOnSurface(F f)    
    {
        makeCurrent();
        visual::with_current_context([&](QOpenGLFunctions& _)
        {
          _.glViewport(0,0, width(), height());
          _.glClearColor(0.0,0.0,0.0,1.0);
          glMatrixMode(GL_PROJECTION);
          glLoadIdentity();
          QMatrix4x4 _m;
          QRectF&& _rect = viewRect();
          _m.ortho(_rect.left(),_rect.right(),_rect.top(),_rect.bottom(),-1.0,1.0);
          glMultMatrixf(_m.constData());

          glMatrixMode(GL_MODELVIEW);
          glLoadIdentity();
          f(_);
        });
    }

    void TuningGLView::drawWarpGrid()
    {
      updateWarpBuffer();

      drawOnSurface([&](QOpenGLFunctions& _)
      {
        _.glBindTexture(GL_TEXTURE_2D, warpGridBuffer_->texture());
        _.glDisable(GL_LIGHTING);
        _.glEnable(GL_BLEND);
        tuning_->drawWarpGrid();

        _.glBindTexture(GL_TEXTURE_2D, 0.0);

        drawScreenBorder();
      });
    }

    void TuningGLView::drawBlendMask()
    {
      updateWarpBuffer();

      drawOnSurface([&](QOpenGLFunctions& _)
      {
        auto& _blendMask = tuning()->blendMask();

        _.glBindTexture(GL_TEXTURE_2D, warpGridBuffer_->texture());
        _.glDisable(GL_LIGHTING);
        _.glEnable(GL_BLEND);
        tuning_->drawBlendMask();

        _.glBindTexture(GL_TEXTURE_2D, 0.0);

        drawScreenBorder();
      });
    }
      
    void TuningGLView::drawScreenBorder()
    {
        // Draw screen border rectangle on top
        if (!viewOnly())
        {
          glPolygonMode(GL_FRONT,GL_LINE);
          glPolygonMode(GL_BACK,GL_LINE);
          auto _color = tuning()->color();
          glColor3f(_color.redF(),_color.greenF(),_color.blueF());
          visual::Rectangle::draw();
          glPolygonMode(GL_FRONT,GL_FILL);
          glPolygonMode(GL_BACK,GL_FILL);
        }
    }

    void TuningGLView::paintGL()
    {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      if (!tuning_ || !session_) return;

      if (!isDrawingEnabled())
        if (frameBuffer_)
        {
          frameBuffer_->bind();
          glViewport(0, 0, (GLint)frameBuffer_->width(), (GLint)frameBuffer_->height());
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

      glDisable(GL_BLEND);

      session_->update();

      glPushAttrib(GL_ALL_ATTRIB_BITS);
      switch (session()->mode())
      {
      case Session::Mode::SCREENSETUP:
        //tuning_.drawScreenImage();
        break;
      case Session::Mode::PROJECTIONSETUP:
        drawCanvas();
        break;
      case Session::Mode::WARP:
        drawWarpGrid();
        break;
      case Session::Mode::BLEND:
        drawBlendMask();
        break;
      case Session::Mode::EXPORT:
        break;
      }
      glPopAttrib();

      if (!isDrawingEnabled())
      {
        if (frameBuffer_)
        {
          frameBuffer_->release();
          makeCurrent();

          glViewport(0,0, width(), height());
          glBindTexture(GL_TEXTURE_2D, frameBuffer_->texture());

          /// Setup orthogonal projection
          glMatrixMode(GL_PROJECTION);
          {
            glLoadIdentity();
            QMatrix4x4 _m;
            _m.ortho(-0.5,0.5,-0.5,0.5,-1.0,1.0);
            glMultMatrixf(_m.constData());
          }
          glDisable( GL_CULL_FACE );
          glEnable(GL_TEXTURE_2D);
          glMatrixMode(GL_MODELVIEW);

          glLoadIdentity();
          glActiveTexture(GL_TEXTURE0);

          glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
          glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

          glBegin(GL_QUADS);
          {
            glTexCoord2f(0.0f,0.0f);
            glVertex2f(-0.5f,-0.5f);
            glTexCoord2f(1.0f,0.0f);
            glVertex2f(0.5f,-0.5f);
            glTexCoord2f(1.0f,1.0f);
            glVertex2f(0.5f,0.5f);
            glTexCoord2f(0.0f,1.0f);
            glVertex2f(-0.5f,0.5f);
          }
          glEnd();

          glBindTexture(GL_TEXTURE_2D, 0);

          glEnable(GL_BLEND);

        }
      }
    }
  }
}
