#ifndef  _DIALOGMAKEEXTRUSION_H_
#define _DIALOGMAKEEXTRUSION_H_

#include "moduleBase/ModuleType.h"
#include "geoDialogBase.h"
#include <QMultiHash>
#include "geoPointWidget.h"


class vtkActor;
class TopoDS_Shape;

namespace Ui
{
	class CreateExtrusion;
}
namespace Geometry
{
	class GeometrySet;
}
namespace GeometryWidget
{
	class GEOMETRYWIDGETSAPI CreateExtrusionDialog : public GeoDialogBase
	{
		Q_OBJECT
	public:
		CreateExtrusionDialog(GUI::MainWindow* m, MainWidget::PreWindow* p);
		CreateExtrusionDialog(GUI::MainWindow* m, MainWidget::PreWindow* p,Geometry::GeometrySet* set);
		~CreateExtrusionDialog();

	private slots:
		void selectActorShape(vtkActor* ac, int shape, Geometry::GeometrySet* set) override;
		void on_geoSelectCurve_clicked();
		void on_radioButtonUser();

	private:
		void init();
		void closeEvent(QCloseEvent *);
		void accept() override;
		void reject() override;


	private:
		Ui::CreateExtrusion* _ui{};
		QList<vtkActor*> _actors{};

		double _distance{1.0};
		QString _extruName{};
		QMultiHash<Geometry::GeometrySet*, int> _shapeHash{};




	};



}
















































#endif 