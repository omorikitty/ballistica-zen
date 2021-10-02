# Released under the MIT License. See LICENSE for details.
#
"""Functionality related to capturing nested dataclass paths."""

from __future__ import annotations

import dataclasses
from typing import TYPE_CHECKING, TypeVar, Generic

from efro.dataclassio._base import _parse_annotated, _get_origin
from efro.dataclassio._prep import PrepSession

if TYPE_CHECKING:
    from typing import Any, Dict, Type, Tuple, Optional, List, Set, Callable

T = TypeVar('T')


class _PathCapture:
    """Utility for obtaining dataclass storage paths in a type safe way."""

    def __init__(self, obj: Any, pathparts: List[str] = None):
        self._is_dataclass = dataclasses.is_dataclass(obj)
        if pathparts is None:
            pathparts = []
        self._cls = obj if isinstance(obj, type) else type(obj)
        self._pathparts = pathparts

    def __getattr__(self, name: str) -> _PathCapture:

        # We only allow diving into sub-objects if we are a dataclass.
        if not self._is_dataclass:
            raise TypeError(
                f"Field path cannot include attribute '{name}' "
                f'under parent {self._cls}; parent types must be dataclasses.')

        prep = PrepSession(explicit=False).prep_dataclass(self._cls,
                                                          recursion_level=0)
        try:
            anntype = prep.annotations[name]
        except KeyError as exc:
            raise AttributeError(f'{type(self)} has no {name} field.') from exc
        anntype, ioattrs = _parse_annotated(anntype)
        storagename = (name if (ioattrs is None or ioattrs.storagename is None)
                       else ioattrs.storagename)
        origin = _get_origin(anntype)
        return _PathCapture(origin, pathparts=self._pathparts + [storagename])

    @property
    def path(self) -> str:
        """The final output path."""
        return '.'.join(self._pathparts)


class DataclassFieldLookup(Generic[T]):
    """Get info about nested dataclass fields in type-safe way."""

    def __init__(self, cls: Type[T]) -> None:
        self.cls = cls

    def path(self, callback: Callable[[T], Any]) -> str:
        """Look up a path on child dataclass fields.

        example:
          DataclassFieldLookup(MyType).path(lambda obj: obj.foo.bar)

        The above example will return the string 'foo.bar' or something
        like 'f.b' if the dataclasses have custom storage names set.
        It will also be static-type-checked, triggering an error if
        MyType.foo.bar is not a valid path. Note, however, that the
        callback technically allows any return value but only nested
        dataclasses and their fields will succeed.
        """

        # We tell the type system that we are returning an instance
        # of our class, which allows it to perform type checking on
        # member lookups. In reality, however, we are providing a
        # special object which captures path lookups so we can build
        # a string from them.
        if not TYPE_CHECKING:
            out = callback(_PathCapture(self.cls))
            if not isinstance(out, _PathCapture):
                raise TypeError(f'Expected a valid path under'
                                f' the provided object; got a {type(out)}.')
            return out.path
        return ''