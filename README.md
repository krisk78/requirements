# requirements header-only library

Implements the class Requirements<br>
Class Requirements implements a framework to handle dependencies between objects<br>
Reflexivity can be set at instantiation (disabled by default)

## Constructor

### Requirements(const bool reflexive)
Instantiate an instance of the class and sets its immutable reflexivity status (disabled by default)

## Information

### bool reflexive() const
Returns the reflexive status of the instance

### bool empty() const
Returns true if no dependencies are set

### size_t size() const
Returns the number of dependencies

### bool exists(const &lt;T&gt;& dependent, const <T>& requirement) const
Returns true if a direct dependency between the two given objects exists in the given direction

### bool requires(const &lt;T&gt;& dependent, const <T>& requirement) const
Returns true if a direct or indirect dependency exists between the given objects in the given direction

### bool has_requirements(const &lt;T&gt;& dependent) const
Returns true if the given object depends on at least one other object

### bool has_dependents(const &lt;T&gt;& requirement) const
Returns true if the given object is required by at least one other object

## CRUD methods

### void clear()
Removes all dependencies

### void add(const &lt;T&gt;& dependent, const &lt;T&gt;& requirement)
Adds a dependency between the given dependent and requirement<br>
An assertion occurs if:
* dependent and requirement are the same object
* the relation already exists
* the opposite relation already exists while reflexivity is not active

### void remove(const &lt;T&gt;& dependent, const &lt;T&gt;& requirement)
Removes the given dependency between the given dependent and requirement

### void remove_dependent(const &lt;T&gt;& dependent)
Removes all dependencies involving the given object as a dependent

### void remove_requirement(const &lt;T&gt;& requirement)
Removes all dependencies involving the given object as a requirement

### void remove_all(const &lt;T&gt;& object)
Removes all dependencies involving the given object as both requirement or dependent

### std::vector&lt;T&gt; requirements(const &lt;T&gt;& dependent) const
Returns the list of objects on which the given object directly depends

### std::vector&lt;T&gt; dependents(const &lt;T&gt;& requirement) const
Returns the list of objects that are directly dependent of the given object

### std::vector&lt;std::vector&lt;T&gt;&gt; all_requirements(const &lt;T&gt;& dependent) const
Lists all objects for which the one given is dependent, directly or indirectly.

### std::vector&lt;std::vector&lt;T&gt;&gt; all_dependencies(const &lt;T&gt;& requirement) const
Lists all objects that are dependent from the given object, directly or indirectly.

### std::vector&lt;std::vector&lt;T&gt;&gt; all_requirements(bool without_duplicates) const
Lists all dependencies by branch, from dependents to required objects<br>
If without_duplicates is true, then only objects that have no dependents are used as root of branches

### std::vector&lt;std::vector&lt;T&gt;&gt; all_dependencies(bool without_duplicates) const
Lists all dependencies by branch, from requirements to dependents<br>
If without_duplicates is true, then only objects that not depends on any object are used as root of branches

### std::unordered_multimap&lt;T, T&gt; get() const
Lists all pairs of objects (dependent, requirement)

### void set(const std::unordered_multimap&lt;T, T&gt; dependecies)
Resets dependencies from the given list of pairs (dependent, requirement)<br>
An assertion occurs if rules are broken, see add method

### void merge(const std::unordered_multimap&lt;T, T&gt; dependencies)
Merges the given list of pairs (dependent, requirement) with existing ones<br>
An assertion occurs if rules are broken, see add method
