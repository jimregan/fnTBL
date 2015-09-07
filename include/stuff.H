template <class Key, class Value, class __trie_node_type> template <class InputIterator> 
typename trie<Key, Value, __trie_node_type>::const_iterator trie<Key, Value, __trie_node_type>::find(InputIterator start, InputIterator end) const
{
  link_type node = root, parent = 0;
  //   __trie_node_base_iterator::iterator_stack_type s;
  const_iterator it;

  for(InputIterator i=start ; i != end ; ++i) { // for each key in the sequence
    if (node==0) // At the previous step, the value did not have any continuations
      return this->end();
    
    typename node_type::value_iterator j = node->values_find(*i);
    if(j == node->values_end()) // the key was not found
      return this->end();
    
    it.value_iters.push_back(j);
    parent = node;
    
    if (i!=end-1)
      node = static_cast<link_type>(node->child_on_pos(j-node->values_begin()));
  }
  
  // As the node of the iterator, the last visited node will be assigned.
  it.node = parent;
  
  return it;
}
