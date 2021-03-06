#include "selectionwindow.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/select.h"
#include "palettebox.h"
#include "scoreaccessibility.h"
namespace Ms {

static const char* labels[] = {
      QT_TRANSLATE_NOOP("selectionfilter", "Voice 1"),
      QT_TRANSLATE_NOOP("selectionfilter", "Voice 2"),
      QT_TRANSLATE_NOOP("selectionfilter", "Voice 3"),
      QT_TRANSLATE_NOOP("selectionfilter", "Voice 4"),
      QT_TRANSLATE_NOOP("selectionfilter", "Dynamics"),
      QT_TRANSLATE_NOOP("selectionfilter", "Fingering"),
      QT_TRANSLATE_NOOP("selectionfilter", "Lyrics"),
      QT_TRANSLATE_NOOP("selectionfilter", "Chord Symbols"),
      QT_TRANSLATE_NOOP("selectionfilter", "Other Text"),
      QT_TRANSLATE_NOOP("selectionfilter", "Articulations"),
      QT_TRANSLATE_NOOP("selectionfilter", "Slurs"),
      QT_TRANSLATE_NOOP("selectionfilter", "Figured Bass"),
      QT_TRANSLATE_NOOP("selectionfilter", "Ottava"),
      QT_TRANSLATE_NOOP("selectionfilter", "Pedal Lines"),
      QT_TRANSLATE_NOOP("selectionfilter", "Other Lines"),
      QT_TRANSLATE_NOOP("selectionfilter", "Arpeggios"),
      QT_TRANSLATE_NOOP("selectionfilter", "Glissandi"),
      QT_TRANSLATE_NOOP("selectionfilter", "Fretboard Diagrams"),
      QT_TRANSLATE_NOOP("selectionfilter", "Breath Marks"),
      QT_TRANSLATE_NOOP("selectionfilter", "Tremolo"),
      QT_TRANSLATE_NOOP("selectionfilter", "Grace Notes")
      };

const int numLabels = sizeof(labels)/sizeof(labels[0]);

SelectionListWidget::SelectionListWidget(QWidget *parent) : QListWidget(parent)
      {
      setAccessibleName(tr("Selection filter"));
      setAccessibleDescription(tr("Use Tab and Backtab to move through the check boxes"));
      setFrameShape(QFrame::NoFrame);
      setSelectionMode(QAbstractItemView::SingleSelection);
      setFocusPolicy(Qt::TabFocus);
      setTabKeyNavigation(true);

      for (int row = 0; row < numLabels; row++) {
            QListWidgetItem *listItem = new QListWidgetItem(qApp->translate("selectionfilter", labels[row]),this);
            listItem->setData(Qt::UserRole, QVariant(1 << row));
            listItem->setData(Qt::AccessibleTextRole, qApp->translate("selectionfilter", labels[row]));
            listItem->setCheckState(Qt::Unchecked);
            addItem(listItem);
            }
      }
void SelectionListWidget::focusInEvent(QFocusEvent* e) {
      setCurrentRow(0);
      QListWidget::focusInEvent(e);
      }

SelectionWindow::SelectionWindow(QWidget *parent, Score* score) :
      QDockWidget(tr("Selection"),parent)
      {
      setObjectName("selection-window");
      setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
      _score = score;

      _listWidget = new SelectionListWidget;
      setWidget(_listWidget);

      updateFilteredElements();
      connect(_listWidget, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(changeCheckbox(QListWidgetItem*)));
      }

SelectionWindow::~SelectionWindow()
      {
      QSettings settings;
      if (isVisible()) {
            settings.setValue("selectionWindow/pos", pos());
            }
      }

void SelectionWindow::updateFilteredElements()
      {
      if (!_score)
            return;
      int filter = _score->selectionFilter().filtered();
      for(int row = 0; row < _listWidget->count(); row++) {
            QListWidgetItem *item = _listWidget->item(row);
            if (filter & 1 << row)
                  item->setCheckState(Qt::Checked);
            else
                  item->setCheckState(Qt::Unchecked);
            }
      }

void SelectionWindow::changeCheckbox(QListWidgetItem* item)
      {
      int type = item->data(Qt::UserRole).toInt();

      bool set = false;
      item->checkState() == Qt::Checked ? set = true : set = false;
      _score->selectionFilter().setFiltered(static_cast<SelectionFilterType>(type),set);

      if (_score->selection().isRange())
            _score->selection().updateSelectedElements();
      updateFilteredElements();
      _score->setUpdateAll();
      _score->end();
      ScoreAccessibility::instance()->updateAccessibilityInfo();
      }

//---------------------------------------------------------
//   showMixer
//---------------------------------------------------------

void MuseScore::showSelectionWindow(bool val)
      {
      QAction* a = getAction("toggle-selection-window");
      if (selectionWindow == 0) {
            selectionWindow = new SelectionWindow(this,this->currentScore());
            connect(selectionWindow, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
            addDockWidget(Qt::LeftDockWidgetArea,selectionWindow);
            if (paletteBox && paletteBox->isVisible()) {
                  tabifyDockWidget(paletteBox, selectionWindow);
                  }
            }
      selectionWindow->setVisible(val);
      if (val) {
            selectionWindow->raise();
            }
      }
void SelectionWindow::closeEvent(QCloseEvent* ev)
      {
      emit closed(false);
      QWidget::closeEvent(ev);
      }

void SelectionWindow::hideEvent(QHideEvent* ev)
      {
      //QSettings settings;
      QSettings settings;
      settings.setValue("selectionWindow/pos", pos());
      QWidget::hideEvent(ev);
      }

void SelectionWindow::setScore(Score* score)
      {
      _score = score;
      updateFilteredElements();
      }

QSize SelectionWindow::sizeHint() const
      {
      return QSize(170 * guiScaling, 170 * guiScaling);
      }

}
