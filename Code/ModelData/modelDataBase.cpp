#include "modelDataBase.h"
#include <QDataStream>
#include <QDomDocument>
#include <QDomElement>
#include <QDomAttr>
#include <assert.h>
#include <QTextStream>
#include <QDir>
#include <QMessageBox>
#include "settings/busAPI.h"
#include "BCBase/BCBase.h"
#include "BCBase/BCPressure.h"
#include "BCBase/BCTemperature.h"
#include "BCBase/BCDisplacement.h"
#include "BCBase/BCUserDef.h"
#include "ModelData/simulationSettingBase.h"
#include "ModelData/solverSettingBase.h"
#include "ConfigOptions/ConfigOptions.h"
#include "ConfigOptions/DataConfig.h"
#include <QDebug>
#include <assert.h>
#include "python/PyAgent.h"
namespace ModelData
{
    int ModelDataBase::maxID = 0;
    ModelDataBase::ModelDataBase(ProjectTreeType treeType)
        : _treeType(treeType)
    {
        maxID++;
        setID(maxID);
        this->setModuleType(DataProperty::Module_Model);
        _simlutationSetting = new SimlutationSettingBase(this);
        _solverSetting = new SolverSettingBase(this);
    }
    ModelDataBase::~ModelDataBase()
    {
        int n = _bcList.size();
        for (int i = 0; i < n; ++i)
        {
            BCBase::BCBase* bc = _bcList.at(i);
            delete bc;
        }
        _bcList.clear();
    }
    void ModelDataBase::resetMaxID()
    {
        maxID = 0;
    }
    int ModelDataBase::getMaxID()
    {
        return maxID;
    }
    void ModelDataBase::setTreeType(ProjectTreeType type)
    {
        _treeType = type;
    }
    ProjectTreeType ModelDataBase::getTreeType()
    {
        return _treeType;
    }
    void ModelDataBase::dataToStream(QDataStream* datas)
    {
        *datas << _name << _id << _treeType;
        DataBase::dataToStream(datas);
    }
    QDomElement& ModelDataBase::writeToProjectFile(QDomDocument* doc, QDomElement* parent)
    {
        QDomElement modelEle = doc->createElement("Model");
        QDomAttr idattr = doc->createAttribute("ID");
        idattr.setValue(QString::number(_id));
        modelEle.setAttributeNode(idattr);
        QDomAttr typeAttr = doc->createAttribute("Type");
        typeAttr.setValue(getTreeTypeToSring(_treeType));
        modelEle.setAttributeNode(typeAttr);
        QDomElement name = doc->createElement("Name");
        QDomText nametext = doc->createTextNode(_name);
        name.appendChild(nametext);
        modelEle.appendChild(name);

//		DataBase::writeParameters(doc, &modelEle);
        const int ngeo = _geometryList.size();
        if (ngeo > 0)
        {
            QDomElement compEle = doc->createElement("GeometrySet");
            QDomText domtext = doc->createTextNode("ID");
            QString text;
            for (int i = 0; i < ngeo; ++i)
                text.append(QString("%1,").arg(_geometryList.at(i)));
            text.resize(text.size() - 1);
            domtext.setData(text);
            compEle.appendChild(domtext);
            modelEle.appendChild(compEle);
        }


        //Set
        const int nset = _componentIDList.size();
        if (nset > 0)
        {
            QDomElement compEle = doc->createElement("Component");
            QDomText domtext = doc->createTextNode("ID");
            QString text;
            for (int i = 0; i < nset; ++i)
                text.append(QString("%1,").arg(_componentIDList.at(i)));
            text.resize(text.size() - 1);
            domtext.setData(text);
            compEle.appendChild(domtext);
            modelEle.appendChild(compEle);
        }
        //BC
        const int nbc = _bcList.size();
        if (nbc > 0)
        {
            QDomElement bcelememnt = doc->createElement("BoundaryCondition");

            for (int i = 0; i < nbc; ++i)
            {
                BCBase::BCBase* bc = _bcList.at(i);
                QDomElement bcele = doc->createElement("BC");
                bc->writeToProjectFile(doc, &bcele);
                bcelememnt.appendChild(bcele);
            }
            modelEle.appendChild(bcelememnt);
        }
        
        QDomElement simEle = doc->createElement("SimlutationSetting");
        _simlutationSetting->writeToProjectFile(doc, &simEle);
        modelEle.appendChild(simEle);

        QDomElement solverEle = doc->createElement("SolverSetting");
        _solverSetting->writeToProjectFile(doc, &solverEle);
        modelEle.appendChild(solverEle);

        parent->appendChild(modelEle);
        return modelEle;
    }
    void ModelDataBase::setID(int id)
    {
        DataBase::setID(id);
        if (id > maxID)
            maxID = id;
    }
    ProjectTreeType ModelDataBase::getTreeTypeByString(const QString& stype)
    {
        ProjectTreeType type = UnDefined;

        QString index = stype.toLower().remove("type");
        bool ok = false;
        int i = index.toInt(&ok);
        if (!ok)
        {
            assert(0);
            return type;
        }
        return  ProjectTreeType(TreeType + i);
    }
    QString ModelDataBase::getTreeTypeToSring(ProjectTreeType type)
    {
        QString stype = "UnDefined";
        if ((int)type > 0)
        {
            stype = QString("Type%1").arg(int(type));
        }
        return stype;
    }
    void ModelDataBase::readDataFromProjectFile(QDomElement* e)
    {
        int id = e->attribute("ID").toInt();
        this->setID(id);
        QDomNodeList nodelist = e->elementsByTagName("Name");
        if (nodelist.size() == 1)
        {
            QDomElement nameEle = nodelist.at(0).toElement();
            QString name = nameEle.text();
            this->setName(name);
        }

        QDomNodeList geolist = e->elementsByTagName("GeometrySet");
        if (geolist.size() == 1)
        {
            QDomElement compEle = geolist.at(0).toElement();
            QString idtext = compEle.text();
            QStringList sids = idtext.split(",");
            for (int i = 0; i < sids.size(); ++i)
            {
                int id = sids.at(i).toInt();
                _geometryList.append(id);
            }
        }

        //component
        QDomNodeList complist = e->elementsByTagName("Component");
        if (complist.size() == 1)
        {
            QDomElement compEle = complist.at(0).toElement();
            QString idtext = compEle.text();
            QStringList sids = idtext.split(",");
            for (int i = 0; i < sids.size(); ++i)
            {
                int id = sids.at(i).toInt();
                _componentIDList.append(id);
            }
        }
        
        //BC
        QDomNodeList bclist = e->elementsByTagName("BoundaryCondition");
        if (bclist.size() == 1)
        {
            QDomElement bcele = bclist.at(0).toElement();
            QDomNodeList bcelelist = bcele.elementsByTagName("BC");
            const int n = bcelelist.size();
            for (int i = 0; i < n; ++i)
            {
                QDomElement ele = bcelelist.at(i).toElement();
                QString stype = ele.attribute("Type");
                QString t = stype.toLower();
                BCBase::BCBase* bc = nullptr;

                if (t == "pressure")
                    bc = new BCBase::BCPressure;
                else if (t == "temperature")
                    bc = new BCBase::BCTemperature;
                else if (t == "displacement")
                    bc = new BCBase::BCDisplacement;
                else if (t == "userdefine")
                    bc = new BCBase::BCUserDef;
                else if (t == "rotation" || t == "anglevelocity" || t == "angleacceleration")
                    bc = new BCBase::BCScalarBase;
                else if (t == "acceleration")
                    bc = new BCBase::BCVectorBase;

                else bc = new BCBase::BCBase;
                
                if (bc == nullptr) continue;
                bc->readDataFromProjectFile(&ele);
//				_bcList.append(bc);
                this->appeendBC(bc);
                bc->generateParaInfo();
            }
        }
        //Simlutation setting
        QDomNodeList simSetting = e->elementsByTagName("SimlutationSetting");
        if (simSetting.size() == 1)
        {
            QDomElement simele = simSetting.at(0).toElement();
            _simlutationSetting->readDataFromProjectFile(&simele);
        }
        QDomNodeList solverSetting = e->elementsByTagName("SolverSetting");
        if (solverSetting.size() == 1)
        {
            QDomElement solverele = solverSetting.at(0).toElement();
            _solverSetting->readDataFromProjectFile(&solverele);
        }
    }

    void ModelDataBase::writeToSolverText(QTextStream* stream)
    {
        ///<MG for wave resistance
// 		if (_treeType != TreeType + 1){ return; }
// 		*stream << "船型标识符(os表示油船，cs表示集装箱船，bs表示散货船)：" << endl;
    }
    void ModelDataBase::writeToSolverXML(QDomDocument* doc,QDomElement* e)
    {
//		Q_UNUSED(doc);
        writeToProjectFile(doc, e);
    }
    QString ModelDataBase::getPath()
    {
        QString workingdir = Setting::BusAPI::instance()->getWorkingDir();
        QDir dir;
        if ((workingdir.isEmpty()) || (!dir.exists(workingdir)))
        {
            QMessageBox::warning(nullptr, QObject::tr("Warning"), QObject::tr("Wrong Working Directory !"));
            return "";
        }
        QString path = workingdir + "/" + _name;
        if (!dir.exists(path))
            dir.mkdir(path);
        return path;
    }

    bool ModelDataBase::checkSolveableStatus(QVector<ModuleBase::Message> & messages)
    {
        return true;
    }

    void ModelDataBase::setSolveTime(double t)
    {
        _solvetime = t;
    }
    double ModelDataBase::getSolveTime()
    {
        return _solvetime;
    }
    void ModelDataBase::setOutputFileName(QString name)
    {
        _outputFileName = name;
//		qDebug() << name;
    }
    QString ModelDataBase::getOutputFileName()
    {
        return _outputFileName;
    }
    int ModelDataBase::getBCCount()
    {
        return _bcList.size();
    }
    void ModelDataBase::appeendBC(BCBase::BCBase* bc)
    {
        int index = _bcList.size();
        _bcList.append(bc);
        bc->setIndex(index);
        bc->setID(_id);
    }
    QList<BCBase::BCBase*> ModelDataBase::getBCByType(BCBase::BCType t)
    {
        QList<BCBase::BCBase*> l;
        const int n = _bcList.size();
        for (int i = 0; i < n; ++i)
        {
            BCBase::BCBase* bc = _bcList.at(i);
            if (bc->getType() == t)
                l.append(bc);
        }
        return l;
    }
    BCBase::BCBase* ModelDataBase::getBCAt(const int index)
    {
        if(index >= 0 && index < _bcList.size())
           return _bcList.at(index);
         return nullptr;
    }
    void ModelDataBase::removeBCAt(const int index)
    {
        assert(index >= 0 && index < _bcList.size());
        _bcList.removeAt(index);
    }
    void ModelDataBase::setMeshSetList(QList<int> ids)
    {
        _componentIDList = ids;
        //Py::PythonAagent::getInstance()->unLock();
    }
    QList<int> ModelDataBase::getMeshSetList()
    {
        return _componentIDList;
    }
    void ModelDataBase::setMeshKernelList(QList<int> k)
    {
        _meshKernalIDList = k;
    }
    QList<int> ModelDataBase::getMeshKernalList()
    {
        return _meshKernalIDList;
    }
    void ModelDataBase::removeMeshSetAt(int index)
    {
        assert(index >= 0 && index < _componentIDList.size());
        int compid = _componentIDList.at(index);
        _componentIDList.removeAt(index);

        QList<BCBase::BCBase*> bcList{};
        for (int i = 0; i < _bcList.size(); ++i)
        {
            BCBase::BCBase* bc = _bcList.at(i);
            if (bc->getMeshSetID() == compid)
                bcList.append(bc);
        }
        for (int i = 0; i < bcList.size(); ++i)
        {
            BCBase::BCBase* bc = bcList.at(i);
            _bcList.removeOne(bc);
            delete bc;
        }
    }
    SimlutationSettingBase* ModelDataBase::getSimlutationSetting()
    {
        return _simlutationSetting;
    }
    SolverSettingBase* ModelDataBase::getSolverSetting()
    {
        return _solverSetting;
    }
    void ModelDataBase::writeToProjectFile1(QDomDocument* doc, QDomElement* modelEle)
    {
    //	QDomElement modelEle = doc->createElement("Model");
        QDomAttr idattr = doc->createAttribute("ID");
        idattr.setValue(QString::number(_id));
        modelEle->setAttributeNode(idattr);
        QDomAttr typeAttr = doc->createAttribute("Type");
        typeAttr.setValue(getTreeTypeToSring(_treeType));
        modelEle->setAttributeNode(typeAttr);
        QDomElement name = doc->createElement("Name");
        QDomText nametext = doc->createTextNode(_name);
        name.appendChild(nametext);
        modelEle->appendChild(name);

        const int ngeo = _geometryList.size();
        if (ngeo > 0)
        {
            QDomElement compEle = doc->createElement("GeometrySet");
            QDomText domtext = doc->createTextNode("ID");
            QString text;
            for (int i = 0; i < ngeo; ++i)
                text.append(QString("%1,").arg(_geometryList.at(i)));
            text.resize(text.size() - 1);
            domtext.setData(text);
            compEle.appendChild(domtext);
            modelEle->appendChild(compEle);
        }

        //Set
        const int nset = _componentIDList.size();
        if (nset > 0)
        {
            QDomElement compEle = doc->createElement("Component");
            QDomText domtext = doc->createTextNode("ID");
            QString text;
            for (int i = 0; i < nset; ++i)
                text.append(QString("%1,").arg(_componentIDList.at(i)));
            text.resize(text.size() - 1);
            domtext.setData(text);
            compEle.appendChild(domtext);
            modelEle->appendChild(compEle);
        }
        //BC
        const int nbc = _bcList.size();
        if (nbc > 0)
        {
            QDomElement bcelememnt = doc->createElement("BoundaryCondition");

            for (int i = 0; i < nbc; ++i)
            {
                BCBase::BCBase* bc = _bcList.at(i);
                QDomElement bcele = doc->createElement("BC");
                bc->writeToProjectFile(doc, &bcele);
                bcelememnt.appendChild(bcele);
            }
            modelEle->appendChild(bcelememnt);
        }

        QDomElement simEle = doc->createElement("SimlutationSetting");
        _simlutationSetting->writeToProjectFile(doc, &simEle);
        modelEle->appendChild(simEle);

        QDomElement solverEle = doc->createElement("SolverSetting");
        _solverSetting->writeToProjectFile(doc, &solverEle);
        modelEle->appendChild(solverEle);
    }
    void ModelDataBase::copyFormConfig()
    {
        ConfigOption::DataConfig* dataconfig = ConfigOption::ConfigOption::getInstance()->getDataConfig();
        if (dataconfig == nullptr) return;
        DataProperty::DataBase* sim = dataconfig->getSimlutationData(_treeType);
        if (sim != nullptr) _simlutationSetting->copy(sim);

        DataProperty::DataBase* solver = dataconfig->getSolverSettingData(_treeType);
        if (solver != nullptr) _solverSetting->copy(solver);
    }
    bool ModelDataBase::isComponentUsed(int index)
    {
        if (index < 0 || index >= _componentIDList.size()) return false;

        const int id = _componentIDList.at(index);
        for(int i = 0; i < _bcList.size(); ++i)
        {
            BCBase::BCBase* bc = _bcList.at(i);
            if (bc->getMeshSetID() == id)
                return true;
        }

        return false;
    }

    DataProperty::ParameterBase* ModelDataBase::getParameterByName(QString name)
    {
        return nullptr;
    }

    QList<int> ModelDataBase::getGeometryList()
    {
        return _geometryList;
    }

    void ModelDataBase::setGeometryList(QList<int> geo)
    {
        _geometryList = geo;
    }

    void ModelDataBase::generateParaInfo()
    {
        _solverSetting->generateParaInfo();
        _simlutationSetting->generateParaInfo();
        DataBase::generateParaInfo();
    }


}