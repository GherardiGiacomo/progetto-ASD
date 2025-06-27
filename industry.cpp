#include "industry.h"
#include <map>
#include <set>
#include <climits>

using namespace std;

namespace industry
{

  struct Item
  {
    string name;
    bool isBasic;
    unsigned quantity;
    list::List dependencies;
    list::List dependents;
  };

  struct st_Industry
  {
    Item *items;
    int size;
    int maxsize;
  };
  // Funzione utile a trovare l'indice di un item nell'array dato il nome
  int findItemIndex(const Industry &indus, const string &name)
  {
    for (int i = 0; i < indus->size; ++i)
    {
      if (indus->items[i].name == name)
        return i;
    }
    return -1;
  }

  // Partizione per il quicksort
  int partition(list::List &l, int low, int high)
  {
    string pivot = list::get(high, l);
    int i = low - 1;

    for (int j = low; j < high; j++)
    {
      if (list::get(j, l) < pivot)
      {
        i++;
        string temp = list::get(i, l);
        list::set(i, list::get(j, l), l);
        list::set(j, temp, l);
      }
    }

    string temp = list::get(i + 1, l);
    list::set(i + 1, list::get(high, l), l);
    list::set(high, temp, l);

    return i + 1;
  }
  void quickSortRec(list::List &l, int low, int high)
  {
    if (low < high)
    {
      int pi = partition(l, low, high);
      quickSortRec(l, low, pi - 1);
      quickSortRec(l, pi + 1, high);
    }
  }

  // Funzione per ordinare con quicksort
  void sortList(list::List &l)
  {
    int n = list::size(l);
    if (n <= 1)
      return;

    quickSortRec(l, 0, n - 1);
  }

  // Verifica se esiste un percorso nel grafo delle dipendenze tra due nodi tramite DFS per evitare cicli
  bool hasPath(const Industry &indus, const string &startNode, const string &endNode, set<string> &visited)
  {
    if (startNode == endNode)
    {
      return true;
    }
    if (visited.count(startNode))
    {
      return false; // Nodo già visitato
    }

    visited.insert(startNode);

    int idx = findItemIndex(indus, startNode);
    if (idx == -1)
    {
      return false; // Nodo non esistente
    }

    list::List dependencies = indus->items[idx].dependencies; // Esplora le dipendenze del nodo corrente
    for (int i = 0; i < list::size(dependencies); ++i)
    {
      string depName = list::get(i, dependencies);
      if (hasPath(indus, depName, endNode, visited))
      {
        return true;
      }
    }
    return false;
  }

  // Funzione ricorsiva per raccogliere tutti gli item che dipendono da un dato item
  void getDeps(const Industry &indus, const string &name, set<string> &resultSet, set<string> &visited)
  {
    if (visited.count(name))
    {
      return;
    }
    visited.insert(name);

    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      return;
    }

    list::List dependents = indus->items[idx].dependents;
    for (int i = 0; i < list::size(dependents); ++i)
    {
      string dependentName = list::get(i, dependents);
      if (resultSet.find(dependentName) == resultSet.end())
      {
        resultSet.insert(dependentName);
      }
      getDeps(indus, dependentName, resultSet, visited);
    }
  }

  Industry createEmptyIndustry()
  {
    Industry ind = new st_Industry;
    ind->size = 0;
    ind->maxsize = 100;
    ind->items = new Item[ind->maxsize];
    return ind;
  }

  bool insertBasicItem(Industry &indus, string name) //
  {
    if (findItemIndex(indus, name) != -1)
    {
      return false; // Item già esistente
    }

    // Gestione del ridimensionamento dinamico dell'array
    if (indus->size == indus->maxsize)
    {
      int newMaxsize = indus->maxsize * 2;
      Item *newItems = new Item[newMaxsize];
      for (int i = 0; i < indus->size; ++i)
      {
        newItems[i] = indus->items[i];
      }
      delete[] indus->items;
      indus->items = newItems;
      indus->maxsize = newMaxsize;
    }

    Item newItem;
    newItem.name = name;
    newItem.isBasic = true;
    newItem.quantity = 0; // Quantità iniziale per basic item
    newItem.dependencies = list::createEmpty();
    newItem.dependents = list::createEmpty();

    indus->items[indus->size++] = newItem;
    return true;
  }

  bool insertItem(Industry &indus, string name, string *components, size_t s)
  {
    if (findItemIndex(indus, name) != -1)
    {
      return false; // Item già esistente
    }
    if (s == 0 || components == nullptr)
    {
      return false; // Un item composto deve avere componenti
    }

    // Verifica l'esistenza di tutte le componenti
    for (size_t i = 0; i < s; ++i)
    {
      if (findItemIndex(indus, components[i]) == -1)
      {
        return false;
      }
    }

    // Controllo per prevenire la formazione di cicli nel grafo delle dipendenze
    // Se una componente può raggiungere il nuovo item, si creerebbe un ciclo
    for (size_t i = 0; i < s; ++i)
    {
      set<string> visited;
      if (hasPath(indus, components[i], name, visited))
      {
        return false;
      }
    }

    // Ridimensionamento se necessario
    if (indus->size == indus->maxsize)
    {
      int newMaxsize = indus->maxsize * 2;
      Item *newItems = new Item[newMaxsize];
      for (int i = 0; i < indus->size; ++i)
      {
        newItems[i] = indus->items[i];
      }
      delete[] indus->items;
      indus->items = newItems;
      indus->maxsize = newMaxsize;
    }

    // Creazione e inserimento del nuovo item
    Item newItem;
    newItem.name = name;
    newItem.isBasic = false;
    newItem.quantity = 0;
    newItem.dependencies = list::createEmpty();
    newItem.dependents = list::createEmpty();

    // Aggiornamento delle relazioni di dipendenza bidirezionali
    for (size_t i = 0; i < s; ++i)
    {
      list::addBack(components[i], newItem.dependencies);
      int compIdx = findItemIndex(indus, components[i]);
      if (compIdx != -1)
      {
        list::addBack(name, indus->items[compIdx].dependents);
      }
    }

    indus->items[indus->size++] = newItem;
    return true;
  }

  bool isPresentItem(const Industry &indus, string name)
  {
    return findItemIndex(indus, name) != -1;
  }

  // funzione che raccoglie ricorsivamente tutti gli item che devono essere rimossi quando si rimuove un item
  void findToDelete(const Industry &indus, const string &nameToRemove, set<string> &toRemove)
  {
    if (toRemove.count(nameToRemove))
    {
      return;
    }

    toRemove.insert(nameToRemove);
    int idx = findItemIndex(indus, nameToRemove);
    if (idx == -1)
    {
      return;
    }

    // Rimozione ricorsiva di tutti gli item dipendenti

    list::List dependents = indus->items[idx].dependents;
    for (int i = 0; i < list::size(dependents); ++i)
    {
      string dependentName = list::get(i, dependents);
      findToDelete(indus, dependentName, toRemove);
    }
  }

  bool removeItem(Industry &indus, string name)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      return false;
    }

    set<string> toRemove;
    findToDelete(indus, name, toRemove);

    // Creazione di un nuovo array senza item da rimuovere
    Item *newItems = new Item[indus->maxsize];
    int newSize = 0;

    for (int i = 0; i < indus->size; ++i)
    {
      if (toRemove.find(indus->items[i].name) == toRemove.end())
      {
        newItems[newSize++] = indus->items[i];
      }
      else
      {
        list::clear(indus->items[i].dependencies);
        list::clear(indus->items[i].dependents);
      }
    }

    for (int i = 0; i < newSize; ++i)
    {
      list::List currentDependencies = newItems[i].dependencies;
      list::List newDependencies = list::createEmpty();
      for (int j = 0; j < list::size(currentDependencies); ++j)
      {
        string depName = list::get(j, currentDependencies);
        if (toRemove.find(depName) == toRemove.end())
        {
          list::addBack(depName, newDependencies);
        }
      }
      list::clear(newItems[i].dependencies);
      newItems[i].dependencies = newDependencies;

      list::List currDependents = newItems[i].dependents;
      list::List newDependents = list::createEmpty();
      for (int j = 0; j < list::size(currDependents); ++j)
      {
        string depName = list::get(j, currDependents);
        if (toRemove.find(depName) == toRemove.end())
        {
          list::addBack(depName, newDependents);
        }
      }
      list::clear(newItems[i].dependents);
      newItems[i].dependents = newDependents;
    }

    delete[] indus->items;
    indus->items = newItems;
    indus->size = newSize;

    return true;
  }

  bool addBasicItem(Industry &indus, string name, int quantity)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1 || !indus->items[idx].isBasic)
    {
      return false;
    }

    if (quantity < 0)
    {
      unsigned int abs_quantity = (unsigned int)(-quantity); // tolgo il - perchè unsigned non può contenere valori negativi
      if (indus->items[idx].quantity < abs_quantity)         // controllo che non vada in negativo
      {
        indus->items[idx].quantity = 0; // se la quantità è insufficiente, imposto a 0
      }
      else
      {
        indus->items[idx].quantity -= abs_quantity; // se la quantità è sufficiente, sottraggo
      }
    }
    else
    {
      indus->items[idx].quantity += quantity; // aggiungo la quantità
    }
    return true;
  }

  bool listNeed(const Industry &indus, string name, list::List &lres)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      lres = list::createEmpty();
      return false;
    }

    lres = list::createEmpty();
    list::List dependencies = indus->items[idx].dependencies;
    for (int i = 0; i < list::size(dependencies); ++i)
    {
      list::addBack(list::get(i, dependencies), lres); // Aggiungo le dipendenze alla lista
    }
    sortList(lres);
    return true;
  }

  bool listNeededBy(const Industry &indus, string name, list::List &lres)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      lres = list::createEmpty();
      return false;
    }

    lres = list::createEmpty();
    list::List dependents = indus->items[idx].dependents;
    for (int i = 0; i < list::size(dependents); ++i)
    {
      list::addBack(list::get(i, dependents), lres);
    }
    sortList(lres);
    return true;
  }

  bool listNeededByChain(const Industry &indus, string name, list::List &lres)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      lres = list::createEmpty();
      return false;
    }

    set<string> resultSet;
    set<string> visited;

    list::List initialDependents = indus->items[idx].dependents;
    for (int i = 0; i < list::size(initialDependents); ++i)
    {
      string dependentName = list::get(i, initialDependents);
      if (resultSet.find(dependentName) == resultSet.end())
      {
        resultSet.insert(dependentName);
      }
      getDeps(indus, dependentName, resultSet, visited);
    }

    lres = list::createEmpty();
    for (set<string>::const_iterator it = resultSet.begin(); it != resultSet.end(); ++it)
    {
      list::addBack(*it, lres);
    }
    sortList(lres);
    return true;
  }

  // Funzione ricorsiva per ottenere i basic items richiesti da un item
  static bool getBasicItem(const Industry &indus, const string &itemName, map<string, unsigned> &requiredBasicItems)
  {
    int idx = findItemIndex(indus, itemName);
    if (idx == -1)
    {
      return false;
    }

    const Item &item = indus->items[idx];

    if (item.isBasic)
    {
      requiredBasicItems[item.name] += 1;
      return true;
    }

    for (int i = 0; i < list::size(item.dependencies); ++i)
    {
      string dependencyName = list::get(i, item.dependencies);
      if (!getBasicItem(indus, dependencyName, requiredBasicItems))
      {
        return false;
      }
    }
    return true;
  }

  bool howManyItem(const Industry &indus, string name, unsigned int &res)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      res = 0;
      return false;
    }

    const Item &item = indus->items[idx];

    if (item.isBasic)
    {
      res = item.quantity;
      return true;
    }

    map<string, unsigned> Map;

    if (!getBasicItem(indus, name, Map))
    {
      res = 0;
      return false;
    }
    unsigned int minProd = UINT_MAX;

    if (Map.empty())
    {
      res = 0;
      return true; // Non ci sono basic items richiesti
    }

    for (map<string, unsigned>::const_iterator it = Map.begin(); it != Map.end(); ++it)
    {
      string basicItemName = it->first;
      unsigned int quantityNeededPerUnit = it->second;

      if (quantityNeededPerUnit == 0)
      {
        continue;
      }

      int basicItemIdx = findItemIndex(indus, basicItemName);
      if (basicItemIdx == -1 || !indus->items[basicItemIdx].isBasic)
      {
        res = 0;
        return true;
      }

      unsigned int availableQuantity = indus->items[basicItemIdx].quantity;
      unsigned int currentProd = availableQuantity / quantityNeededPerUnit;

      if (currentProd < minProd)
      {
        minProd = currentProd;
      }
    }

    res = minProd;
    return true;
  }

}