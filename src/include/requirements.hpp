#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif

/*! \file requirements.hpp
*	\brief Implements the template class Requirements.
*   \author Christophe COUAILLET
*/

#include <cassert>
#include <cstddef>
#include <unordered_map>
#include <vector>

namespace Requirements
{

    /*! \brief Requirements is a class that handles pairs of objects for which the first object depends on the second object.

        Pairs are ensured to be unique.
        By default reflexivity is not allowed (objects that depend on each other) but it can be activated at construction.
        Because it does not have any sense, the reflexivity status can not be changed after instantiation.
    */
    template <typename T>
    class Requirements
    {
    public:

        /*! \brief Default constructor. Set the reflexive status to false.
        */
        Requirements() : Requirements(false) {};

        /*! \brief Constructor. Set the reflexive status to true to allow mutual dependencies.
        *   \param reflexive sets the reflexive mode
        */
        Requirements(const bool reflexive) noexcept
            : m_reflexive(reflexive) {};

        /*! \brief Informs on the reflexive status of the instance.
        *   \return true if reflexive mode is activated
        */
        bool reflexive() const noexcept { return m_reflexive; }

        /*! \brief Clears all dependencies.
        */
        void clear() noexcept { m_requirements.clear(); }

        /*! \brief Checks if the instance contains dependencies.
        *   \return true if no dependency exists
        */
        bool empty() const noexcept { return m_requirements.empty(); }

        /*! \brief Gets the number of dependencies of the instance.
        *   \return the number of dependencies
        */
        size_t size() const noexcept { return m_requirements.size(); }

        void add(const T& dependent, const T& requirement);
        void remove(const T& dependent, const T& requirement);
        void remove_dependent(const T& dependent);
        void remove_requirement(const T& requirement);
        void remove_all(const T& object);
        bool exists(const T& dependent, const T& requirement, bool recurse = false) const;           // check direct dependency
        bool has_requirements(const T& dependent) const noexcept;
        bool has_dependents(const T& requirement) const noexcept;
        std::vector<T> requirements(const T& dependent) const;                                  // lists direct requirements of dependent
        std::vector<T> dependents(const T& requirement) const;                                  // lists direct dependents of requirement
        std::vector<std::vector<T>> all_requirements(const T& dependent) const;                 // returns all requirements of dependent in chains
        std::vector<std::vector<T>> all_dependencies(const T& requirement) const;               // returns all dependencies of requirement in chains
        std::vector<std::vector<T>> all_requirements(bool without_duplicates = true) const;       // returns all chains of requirements
        std::vector<std::vector<T>> all_dependencies(bool without_duplicates = true) const;       // returns all chains of dependencies
        std::unordered_multimap<T, T> get() const;                                          // returns a copy of the table of requirements
        void set(const std::unordered_multimap<T, T>& requirements);                        // initialize the table of requirements with the one provided, performing checks
        void merge(const std::unordered_multimap<T, T>& requirements);                      // append the table provided to the existing table of requirements

    private:
        std::unordered_multimap<T, T> m_requirements{};
        bool m_reflexive{ false };

        bool _requires(const T& dependent, const T& requirement, const T* prev) const;
        //bool _depends(const T& resuirement, const T& dependent, const T* prev) const;
    };

    // Implementation of templates classes and functions

    /*! \brief Add a relation where dependent depends on requirement.
    *   \param dependent the object that depends on the other object
    *   \param requirement the object on which the first object depends
    *   \warning An assertion occurs if:
    *   \li dependent and requirements are the same object,
    *   \li the relation already exists,
    *   \li the opposite relation already exists while reflexivity is not allowed.
    */
    template <typename T>
    void Requirements<T>::add(const T& dependent, const T& requirement)
    {
        assert(!(dependent == requirement) && "A requirement can't be requested for object itself.");
        // we must ensure the implicit requirement does not already exist
        assert(!exists(dependent, requirement, true) && "(Implicit) requirement is already defined.");
        if (!m_reflexive)
            // opposite requirement is only allowed if reflexivity is activated, directly or indirectly
            assert(!exists(requirement, dependent, true) && "Opposite requirement cannot be set while reflexivity is not allowed.");
        m_requirements.insert({ dependent, requirement });
    }

    /*! \brief Removes an existing relation where dependent depends on requirement.
    *   \param dependent,requirement the 2 objects involved in the dependency to remove
    *   \warning An assertion occurs if the relation does not exist.
    *   \warning This function does not remove the opposite relation.
    */
    template <typename T>
    void Requirements<T>::remove(const T& dependent, const T& requirement)
    {
        auto range = m_requirements.equal_range(dependent);
        auto itr = range.first;
        bool found{ false };
        while (itr != range.second)
        {
            if ((*itr).second == requirement)
            {
                itr = m_requirements.erase(itr);
                found = true;
            }
            else
                ++itr;
        }
        assert(found && "Requirement does not exist.");
    }

    /*! \brief Removes all relations involving the object as a dependent.
    *   \param dependent the object that is declared as a dependent in the relations to remove
    *   \warning An assertion occurs if no requirement has been set for this object.
    * 
    *   Relations involving the object as a requirement are left.
    */
    template <typename T>
    void Requirements<T>::remove_dependent(const T& dependent)
    {
        auto range = m_requirements.equal_range(dependent);
        assert(range.first != range.second && "No requirement exists for this argument.");
        m_requirements.erase(range.first, range.second);
    }

    /*! \brief Removes all relations involving the object as a requirement.
    *   \param requirement the object that is declared as a requirement in the relations to remove
    *   \warning An assertion occurs if no dependent has been set for this object.
    * 
    *   Relations involving the object as a dependent are left.
    */
    template <typename T>
    void Requirements<T>::remove_requirement(const T& requirement)
    {
        auto itr = m_requirements.begin();
        bool found{ false };
        while (itr != m_requirements.end())
        {
            if ((*itr).second == requirement)
            {
                itr = m_requirements.erase(itr);
                found = true;
            }
            else
                ++itr;
        }
        assert(found && "No requirement exists for this argument.");
    }

    /*! \brief Removes all existing relations involving the object as a dependent or a requirement.
    *   \param object the object involved as a dependent or a requirement in the relations to remove
    */
    template <typename T>
    void Requirements<T>::remove_all(const T& object)
    {
        if (has_requirements(object))
            remove_dependent(object);
        if (has_dependents(object))
            remove_requirement(object);
    }

    /*! \brief Checks if a direct relationship between the given objects exists.
    *   \param dependent,requirement the 2 objects to check
    *   \param recurse also checks indirect dependencies when set to true, false by default
    *   \return true if a direct relationship exists with the given direction
    *   \sa Requirements< T >::requires()
    */
    template <typename T>
    bool Requirements<T>::exists(const T& dependent, const T& requirement, bool recurse) const
    {
        if (recurse)
            return _requires(dependent, requirement, nullptr);
        bool found{ false };
        auto range = m_requirements.equal_range(dependent);
        auto itr = range.first;
        while (!found && itr != range.second)
        {
            if ((*itr).second == requirement)
                found = true;
            ++itr;
        }
        return found;
    }

    /*! \brief Checks if an object has at least one requirement.
    *   \param dependent the object to check
    *   \return true if at least one requirement has been found for the given object
    */
    template <typename T>
    bool Requirements<T>::has_requirements(const T& dependent) const noexcept
    {
        auto itr = m_requirements.find(dependent);
        return itr != m_requirements.end();
    }

    /*! \brief Checks if an object as at least one dependent.
    *   \param requirement the object to check
    *   \return true if at least one dependent has been found for the given object
    */
    template <typename T>
    bool Requirements<T>::has_dependents(const T& requirement) const noexcept
    {
        bool found{ false };
        auto itr = m_requirements.begin();
        while (!found && itr != m_requirements.end())
        {
            if ((*itr).second == requirement)
                found = true;
            ++itr;
        }
        return found;
    }

    /*! \brief Lists the direct requirements of an object.
    *   \param dependent the object for which direct requirements are searched for
    *   \return the list of its direct requirements
    */
    template <typename T>
    std::vector<T> Requirements<T>::requirements(const T& dependent) const
    {
        std::vector<T> result{};
        auto range = m_requirements.equal_range(dependent);
        auto itr = range.first;
        while (itr != range.second)
        {
            result.push_back((*itr).second);
            ++itr;
        }
        return result;
    }

    /*! \brief Lists the direct dependents of an object.
    *   \param requirement the object for which direct dependents are searched for
    *   \return the list of its direct dependents
    */
    template <typename T>
    std::vector<T> Requirements<T>::dependents(const T& requirement) const
    {
        std::vector<T> result{};
        auto itr = m_requirements.begin();
        while (itr != m_requirements.end())
        {
            if ((*itr).second == requirement)
                result.push_back((*itr).first);
            ++itr;
        }
        return result;
    }

    /*! \brief Lists the branches of objects on which the object depends, directly or indirectly.
    *   \param dependent the object for which direct or indirect requirements are searched for
    *   \return the list of requirement branches starting with the given object
    *   \warning An assertion occurs if the object has no direct requirement.
    */
    template <typename T>
    std::vector<std::vector<T>> Requirements<T>::all_requirements(const T& dependent) const
    {
        assert(has_requirements(dependent) && "No requirement exists for this argument.");
        std::vector<std::vector<T>> result{};
        auto reqs = requirements(dependent);
        for (auto req : reqs)
        {
            std::vector<T> row{ dependent };
            size_t count{ 0 };
            if (has_requirements(req))
            {
                auto subreqs = all_requirements(req);
                for (auto sub : subreqs)
                {
                    if (sub[1] != dependent)         // avoid infinite loop while reflexivity is active
                    {
                        for (auto itr : sub)
                            row.push_back(itr);
                        result.push_back(row);
                        row.clear();
                        row.push_back(dependent);
                        ++count;
                    }
                }
            }
            if (count == 0)
            {
                row.push_back(req);
                result.push_back(row);
            }
        }
        return result;
    }

    /*! \brief Lists the branches of objects that requires the object, directly or indirectly.
    *   \param requirement the object for which direct or indirect dependents are searched for
    *   \return the list of dependent branches starting with the given object
    *   \warning An assertion occurs if the objects has no direct dependent.
    */
    template <typename T>
    std::vector<std::vector<T>> Requirements<T>::all_dependencies(const T& requirement) const
    {
        assert(has_dependents(requirement) && "No dependent exists for this argument.");
        std::vector<std::vector<T>> result{};
        auto deps = dependents(requirement);
        for (auto dep : deps)
        {
            std::vector<T> row{ requirement };
            size_t count{ 0 };
            if (has_dependents(dep))
            {
                auto subdeps = all_dependencies(dep);
                for (auto sub : subdeps)
                {
                    if (sub[1] != requirement)       // avoid infinite loops while reflexivity is active
                    {
                        for (auto itr : sub)
                            row.push_back(itr);
                        result.push_back(row);
                        row.clear();
                        row.push_back(requirement);
                        ++count;
                    }
                }
            }
            if (count == 0)
            {
                row.push_back(dep);
                result.push_back(row);
            }
        }
        return result;
    }

    /*! \brief Lists all branches of dependencies, from dependents to requirements.
    *   \param without_duplicates if true only objects that have no dependents are considered as first element of a branch
    *   \return the list of all branches, from dependents to requirements
    */
    template <typename T>
    std::vector<std::vector<T>> Requirements<T>::all_requirements(bool without_duplicates) const
    {
        std::vector<std::vector<T>> result{};
        auto itr = m_requirements.begin();
        while (itr != m_requirements.end())
        {
            if (!without_duplicates || without_duplicates && !has_dependents((*itr).first))
            {
                bool exists{ false };
                auto res = result.begin();
                while (!exists && res != result.end())
                {
                    if ((*res)[0] == (*itr).first)
                        exists = true;
                    ++res;
                }
                if (!exists)
                {
                    auto deps = all_requirements((*itr).first);
                    for (auto dep : deps)
                        result.push_back(dep);
                }
            }
            ++itr;
        }
        return result;
    }

    /*! \brief Lists all branches of dependencies, from requirements to dependents.
    *   \param without_duplicates if true only objects that not depends on another object are considered as first element of a branch
    *   \return the list of all branches, from requirements to dependents
    */
    template <typename T>
    std::vector<std::vector<T>> Requirements<T>::all_dependencies(bool without_duplicates) const
    {
        std::vector<std::vector<T>> result{};
        auto itr = m_requirements.begin();
        while (itr != m_requirements.end())
        {
            if (!without_duplicates || without_duplicates && !has_requirements((*itr).second))
            {
                bool exists{ false };
                auto res = result.begin();
                while (!exists && res != result.end())
                {
                    if ((*res)[0] == (*itr).second)
                        exists = true;
                    ++res;
                }
                if (!exists)                                // Each requirement must be processed one time to avoid duplicates
                {
                    auto reqs = all_dependencies((*itr).second);
                    for (auto req : reqs)
                        result.push_back(req);
                }
            }
            ++itr;
        }
        return result;
    }

    /*! \brief List all pairs of objects (dependent, requirement).
    *   \return the list of requested pairs
    */
    template <typename T>
    std::unordered_multimap<T, T> Requirements<T>::get() const
    {
        std::unordered_multimap<T, T> result{};
        auto itr = m_requirements.begin();
        while (itr != m_requirements.end())
        {
            result.insert({ (*itr).first, (*itr).second });
            ++itr;
        }
        return result;
    }

    /*! \brief Sets dependencies from the given list. The list of dependencies is first cleared.
    *   \param requirements the list of pairs (dependent, requirement) to create
    *   \warning Controls are performed and an assertion occurs if rules are broken.
    *   \sa Requirements< T >::add()
        \sa Requirements< T >::merge()
    */
    template <typename T>
    void Requirements<T>::set(const std::unordered_multimap<T, T>& requirements)
    {
        clear();
        merge(requirements);
    }

    /*! \brief Adds dependencies from the given list.
    *   \param requirements the list of pairs (dependent, requirement) to add
    *   \warning Controls are performed and an assertion occurs if rules are broken.
    *   \sa Requirements< T >::add()
        \sa Requirements< T >::set()
    */
    template <typename T>
    void Requirements<T>::merge(const std::unordered_multimap<T, T>& requirements)
    {
        auto itr = requirements.begin();
        while (itr != requirements.end())
        {
            add((*itr).first, (*itr).second);
            ++itr;
        }
    }

    template <typename T>
    bool Requirements<T>::_requires(const T& dependent, const T& requirement, const T* prev) const
    {
        bool result = exists(dependent, requirement);
        if (!result && has_requirements(dependent))
        {
            auto reqs = requirements(dependent);
            auto req = reqs.begin();
            while (!result && req != reqs.end())
            {
                if (prev == nullptr || !(*req == *prev))
                    result = _requires(*req, requirement, &dependent);
                ++req;
            }
        }
        return result;
    }

}
