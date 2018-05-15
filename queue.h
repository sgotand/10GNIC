#pragma once

#include <stdint.h>

struct Descriptor {
    phys_addr bufaddr;
    uint64_t  data;
};

class Queue {
    public:
        Queue(size_t descnum, phys_addr physbase, void *virtbase) {
            _descnum = descnum;
            _physbase = physbase;
            _virtbase = virtbase;
            _tail = 0;
        }
        size_t Count() {
            return _descnum;
        }
        phys_addr PhysEnd() {
            return _physbase + sizeof(Descriptor) * _descnum;
        }
        void *VirtEnd() {
            return (void*)((uint8_t*)_virtbase + sizeof(Descriptor) * _descnum);
        }
        ~Queue() {};
    protected:
        Queue();

        size_t _descnum;
        phys_addr _physbase;
        void *_virtbase;
        size_t _tail;
};

class ReceiveQueue : Queue {
    public:
        ReceiveQueue(size_t bufsz, size_t descnum, phys_addr physbase, void *virtbase)
            : Queue(descnum, physbase, virtbase)
        {
            _bufsz = bufsz;

            Descriptor *descs = (Descriptor*)_virtbase;
            phys_addr bufbase = Queue::PhysEnd();
            for(size_t i=0; i<descnum; i++) {
                descs[i].bufaddr = bufbase + i * bufsz;
                descs[i].data = 0;
            }
        }
        phys_addr PhysEnd() {
            return Queue::PhysEnd() + _descnum * _bufsz;
        }
        void *VirtEnd() {
            return (void*)((uint8_t*)Queue::VirtEnd() + _descnum * _bufsz);
        }
    protected:
        size_t _bufsz;
};

class TransmitQueue : Queue {
    public:
        TransmitQueue(size_t descnum, phys_addr physbase, void *virtbase)
            : Queue(descnum, physbase, virtbase)
        {
            Descriptor *descs = (Descriptor*)_virtbase;
            for(size_t i=0; i<descnum; i++) {
                descs[i].data = 0;
            }
            _offset = physbase - (size_t)virtbase;
        }
        void Enqueue(void *buf, size_t sz, bool eop) {
            assert((sz & ((1<<16)-1)) == sz);

            Descriptor *descs = (Descriptor*)_virtbase;

            descs[_tail].bufaddr = (phys_addr)buf + _offset;
            descs[_tail].data = eop ? 1<<24 : 0;
            descs[_tail].data |= sz;

            _tail++, _tail %= _descnum;
        }
    private:
        int64_t _offset;
};
