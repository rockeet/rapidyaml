#ifndef _C4_RYML_DEF_HPP_
#define _C4_RYML_DEF_HPP_

#include "./span.hpp"

namespace c4 {
namespace yml {

#define _c4this  (static_cast< Writer      * >(this))
#define _c4cthis (static_cast< Writer const* >(this))

template< class Writer >
span Emitter< Writer >::emit(Node const* n, bool error_on_excess)
{
    this->_visit(n);
    span result = _c4this->_get(error_on_excess);
    return result;
}

template< class Writer >
size_t Emitter< Writer >::tell() const
{
    return _c4cthis->m_pos;
}

template< class Writer >
void Emitter< Writer >::seek(size_t p)
{
    _c4this->m_pos = p;
}

template< class Writer >
size_t Emitter< Writer >::_visit(Node const* n, size_t ilevel)
{
    if(n->is_stream())
    {
        ;
    }
    _do_visit(n, ilevel);
    if(n->is_stream())
    {
        _write("...\n");
    }
    return _c4this->m_pos;
}

template< class Writer >
void Emitter< Writer >::_do_visit(Node const* n, size_t ilevel, bool indent)
{
    RepC ind{' ', 2 * size_t(indent) * ilevel};

    if(n->is_doc())
    {
        _write("---\n");
    }
    else if(n->is_keyval())
    {
        C4_ASSERT(n->has_parent());
        _write(ind, keysc(n), ": ", valsc(n), '\n');
    }
    else if(n->is_val())
    {
        C4_ASSERT(n->has_parent());
        _write(ind, "- ", valsc(n), '\n');
    }
    else if(n->is_container() && ! n->is_root())
    {
        C4_ASSERT(n->parent_is_map() || n->parent_is_seq());
        C4_ASSERT(n->is_map() || n->is_seq());

        if(n->parent_is_seq())
        {
            C4_ASSERT( ! n->has_key());
            _write(ind, "- ");
        }
        else if(n->parent_is_map())
        {
            C4_ASSERT(n->has_key());
            _write(ind, keysc(n), ':');
        }

        if(n->has_children())
        {
            if(n->is_seq())
            {
                if(n->parent_is_map())
                {
                    _write('\n');
                }
                else
                {
                    // do not indent the first child, as it will be written on the same line
                    indent = false;
                }
            }
            else if(n->is_map())
            {
                if(n->parent_is_seq())
                {
                    // do not indent the first child, as it will be written on the same line
                    indent = false;
                }
                else
                {
                    _write('\n');
                }
            }
            else
            {
                C4_ERROR("invalid node");
            }
        }
        else
        {
            if(n->parent_is_map())
            {
                _write(' ');
            }

            if(n->is_seq())
            {
                _write("[]\n");
            }
            else if(n->is_map())
            {
                _write("{}\n");
            }
        }
    }
    else if(n->is_container() && n->is_root())
    {
        if( ! n->has_children())
        {
            if(n->is_seq())
            {
                _write("[]\n");
            }
            else if(n->is_map())
            {
                _write("{}\n");
            }
        }
    }

    size_t next_level = ilevel + 1;
    if(n->is_stream() || n->is_doc() || n->is_root())
    {
        next_level = ilevel; // do not indent at top level
    }

    for(Node const* ch = n->first_child(); ch; ch = ch->next_sibling())
    {
        _do_visit(ch, next_level, indent);
        indent = true;
    }
}

template< class Writer >
void Emitter< Writer >::_write_one(Scalar const& sc)
{
    const bool no_dquotes = sc.s.first_of( '"') == npos;
    const bool no_squotes = sc.s.first_of('\'') == npos;
    if(no_dquotes && no_squotes)
    {
        if( ! sc.s.empty())
        {
            _c4this->_do_write(sc.s);
        }
        else
        {
            _c4this->_do_write("''");
        }
    }
    else
    {
        if(no_squotes && !no_dquotes)
        {
            _c4this->_do_write('\'');
            _c4this->_do_write(sc.s);
            _c4this->_do_write('\'');
        }
        else if(no_dquotes && !no_squotes)
        {
            _c4this->_do_write('"');
            _c4this->_do_write(sc.s);
            _c4this->_do_write('"');
        }
        else
        {
            size_t pos = 0;
            _c4this->_do_write('\'');
            for(size_t i = 0; i < sc.s.len; ++i)
            {
                if(sc.s[i] == '\'' || sc.s[i] == '\n')
                {
                    cspan sub = sc.s.subspan(pos, i-pos);
                    pos = i;
                    _c4this->_do_write(sub);
                    _c4this->_do_write(sc.s[i]); // write the character twice
                }
            }
            if(pos < sc.s.len)
            {
                cspan sub = sc.s.subspan(pos);
                _c4this->_do_write(sub);
            }
            _c4this->_do_write('\'');
        }
    }
}

#undef _c4this
#undef _c4cthis

} // namespace yml
} // namespace c4

#endif /* _C4_RYML_DEF_HPP_ */
