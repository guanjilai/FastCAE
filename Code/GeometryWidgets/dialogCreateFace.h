#ifndef DIALOG_CREATEFACE_H_
#define DIALOG_CREATEFACE_H_

#include "geoDialogBase.h"
#include <QMultiHash>

class vtkActor;
class TopoDS_Shape;

namespace Ui
{
	class CreateFaceDialog;
}

namespace GeometryWidget
{
	class  GEOMETRYWIDGETSAPI CreateFaceDialog : public GeoDialogBase
	{
		Q_OBJECT
	public:
		CreateFaceDialog(GUI::MainWindow* m, MainWidget::PreWindow* pre);
		CreateFaceDialog(GUI::MainWindow* m, MainWidget::PreWindow* p, Geometry::GeometrySet* set);
		~CreateFaceDialog();


	private slots:
	    void on_geoSelectCurve_clicked();
		void selectActorShape(vtkActor* ac, int shape, Geometry::GeometrySet* set);

	private:
		void init();
		void closeEvent(QCloseEvent *) override;
		void reject() override;
		void accept() override;


	private:
		Ui::CreateFaceDialog* _ui{};

		QList<vtkActor*> _actors{};
		QMultiHash<Geometry::GeometrySet*, int> _shapeHash{};
	};
	
}

#endif