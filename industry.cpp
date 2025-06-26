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

  int findItemIndex(const Industry &indus, const string &name) // Funzione utile a trovare l'indice di un item nell'array dato il nome
  {
    for (int i = 0; i < indus->size; ++i)
    {
      if (indus->items[i].name == name)
        return i;
    }
    return -1;
  }

  int partition(list::List &l, int low, int high) // Partizione per il quicksort
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

  void sortList(list::List &l) // Funzione per ordinare con quicksort
  {
    int n = list::size(l);
    if (n <= 1)
      return;

    quickSortRec(l, 0, n - 1);
  }

 
  bool hasPath(const Industry &indus, const string &startNode, const string &endNode, set<string> &visited) // // Verifica se esiste un percorso nel grafo delle dipendenze tra due nodi tramite DFS per evitare cicli
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

  void collectNeededByChainRec(const Industry &indus, const string &name, set<string> &resultSet, set<string> &visited) // Funzione ricorsiva per raccogliere tutti gli item che dipendono da un dato item
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
      collectNeededByChainRec(indus, dependentName, resultSet, visited);
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

  // Raccoglie ricorsivamente tutti gli item che devono essere rimossi
  // quando si rimuove un item (inclusi tutti i dipendenti)
  void collectItemsToRemove(const Industry &indus, const string &nameToRemove, set<string> &itemsToRemoveSet)
  {
    if (itemsToRemoveSet.count(nameToRemove))
    {
      return; // Già marcato per la rimozione
    }

    itemsToRemoveSet.insert(nameToRemove);
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
      collectItemsToRemove(indus, dependentName, itemsToRemoveSet);
    }
  }

  bool removeItem(Industry &indus, string name)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      return false;
    }

    set<string> itemsToRemoveSet;
    collectItemsToRemove(indus, name, itemsToRemoveSet);

    // Creazione di un nuovo array senza gli item da rimuovere
    Item *newItems = new Item[indus->maxsize];
    int newSize = 0;

    for (int i = 0; i < indus->size; ++i)
    {
      if (itemsToRemoveSet.find(indus->items[i].name) == itemsToRemoveSet.end())
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
        if (itemsToRemoveSet.find(depName) == itemsToRemoveSet.end())
        {
          list::addBack(depName, newDependencies);
        }
      }
      list::clear(newItems[i].dependencies);
      newItems[i].dependencies = newDependencies;

      list::List currentDependents = newItems[i].dependents;
      list::List newDependents = list::createEmpty();
      for (int j = 0; j < list::size(currentDependents); ++j)
      {
        string depName = list::get(j, currentDependents);
        if (itemsToRemoveSet.find(depName) == itemsToRemoveSet.end())
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
      unsigned int abs_quantity = (unsigned int)(-quantity);
      if (indus->items[idx].quantity < abs_quantity)
      {
        indus->items[idx].quantity = 0;
      }
      else
      {
        indus->items[idx].quantity -= abs_quantity;
      }
    }
    else
    {
      indus->items[idx].quantity += quantity;
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
      list::addBack(list::get(i, dependencies), lres);
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
      collectNeededByChainRec(indus, dependentName, resultSet, visited);
    }

    lres = list::createEmpty();
    // Conversione del set in lista ordinata
    for (set<string>::const_iterator it = resultSet.begin(); it != resultSet.end(); ++it)
    {
      list::addBack(*it, lres);
    }
    sortList(lres);
    return true;
  }

  static bool getRequiredBasicItems(const Industry &indus, const string &itemName, map<string, unsigned> &requiredBasicItems)// Funzione ricorsiva per ottenere i basic items richiesti da un item
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

    // Per item composti, accumula ricorsivamente i requisiti di tutte le dipendenze
    for (int i = 0; i < list::size(item.dependencies); ++i)
    {
      string dependencyName = list::get(i, item.dependencies);
      if (!getRequiredBasicItems(indus, dependencyName, requiredBasicItems))
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

    // Per basic items, la quantità producibile è quella disponibile
    if (item.isBasic)
    {
      res = item.quantity;
      return true;
    }

    // Per item composti, calcola i requisiti totali di basic items
    map<string, unsigned> requiredBasicItemsQuantities;

    if (!getRequiredBasicItems(indus, name, requiredBasicItemsQuantities)) 
    {
      res = 0;
      return false;
    }
    unsigned int minProducible = UINT_MAX; 

    if (requiredBasicItemsQuantities.empty())
    {
      res = 0;
      return true; // Non ci sono basic items richiesti
    }

    // Itera su tutti i basic items richiesti e calcola la quantità massima producibile
    for (map<string, unsigned>::const_iterator it = requiredBasicItemsQuantities.begin(); it != requiredBasicItemsQuantities.end(); ++it)
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
      unsigned int currentProducible = availableQuantity / quantityNeededPerUnit;

      if (currentProducible < minProducible)
      {
        minProducible = currentProducible;
      }
    }

    res = minProducible;
    return true;
  }

}