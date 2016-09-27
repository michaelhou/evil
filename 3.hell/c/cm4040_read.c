static ssize_t cm4040_read(struct file *filp, char __user *buf,
                        size_t count, loff_t *ppos)
{
        struct reader_dev *dev = filp->private_data;
        int iobase = dev->p_dev->resource[0]->start;
        size_t bytes_to_read;
        unsigned long i;
        size_t min_bytes_to_read;
        int rc;
        unsigned char uc;

        DEBUGP(2, dev, "-> cm4040_read(%s,%d)\n", current->comm, current->pid);

        if (count == 0)
                return 0;

        if (count < 10)
                return -EFAULT;

        if (filp->f_flags & O_NONBLOCK) {
                DEBUGP(4, dev, "filep->f_flags O_NONBLOCK set\n");
                DEBUGP(2, dev, "<- cm4040_read (failure)\n");
                return -EAGAIN;
        }

        if (!pcmcia_dev_present(dev->p_dev))
                return -ENODEV;

        for (i = 0; i < 5; i++) {
                rc = wait_for_bulk_in_ready(dev);
                if (rc <= 0) {
                        DEBUGP(5, dev, "wait_for_bulk_in_ready rc=%.2x\n", rc);
                        DEBUGP(2, dev, "<- cm4040_read (failed)\n");
                        if (rc == -ERESTARTSYS)
                                return rc;
                        return -EIO;
                }
                dev->r_buf[i] = xinb(iobase + REG_OFFSET_BULK_IN);
#ifdef CM4040_DEBUG
                pr_debug("%lu:%2x ", i, dev->r_buf[i]);
        }
        pr_debug("\n");
#else
        }
#endif
        bytes_to_read = 5 + le32_to_cpu(*(__le32 *)&dev->r_buf[1]);

        DEBUGP(6, dev, "BytesToRead=%zu\n", bytes_to_read);

        min_bytes_to_read = min(count, bytes_to_read + 5);
        min_bytes_to_read = min_t(size_t, min_bytes_to_read, READ_WRITE_BUFFER_SIZE);

        DEBUGP(6, dev, "Min=%zu\n", min_bytes_to_read);

        for (i = 0; i < (min_bytes_to_read-5); i++) {
                rc = wait_for_bulk_in_ready(dev);
                if (rc <= 0) {
                        DEBUGP(5, dev, "wait_for_bulk_in_ready rc=%.2x\n", rc);
                        DEBUGP(2, dev, "<- cm4040_read (failed)\n");
                        if (rc == -ERESTARTSYS)
                                return rc;
                        return -EIO;
                }
                dev->r_buf[i+5] = xinb(iobase + REG_OFFSET_BULK_IN);
#ifdef CM4040_DEBUG
                pr_debug("%lu:%2x ", i, dev->r_buf[i]);
        }
        pr_debug("\n");
#else
        }
#endif

        *ppos = min_bytes_to_read;
        if (copy_to_user(buf, dev->r_buf, min_bytes_to_read))
                return -EFAULT;

        rc = wait_for_bulk_in_ready(dev);
        if (rc <= 0) {
                DEBUGP(5, dev, "wait_for_bulk_in_ready rc=%.2x\n", rc);
                DEBUGP(2, dev, "<- cm4040_read (failed)\n");
                if (rc == -ERESTARTSYS)
                        return rc;
                return -EIO;
        }

        rc = write_sync_reg(SCR_READER_TO_HOST_DONE, dev);
        if (rc <= 0) {
                DEBUGP(5, dev, "write_sync_reg c=%.2x\n", rc);
                DEBUGP(2, dev, "<- cm4040_read (failed)\n");
                if (rc == -ERESTARTSYS)
                        return rc;
                else
                        return -EIO;
        }
        uc = xinb(iobase + REG_OFFSET_BULK_IN);

        DEBUGP(2, dev, "<- cm4040_read (successfully)\n");
        return min_bytes_to_read;
}
