void shift_n_push(int i, int argc, /*const*/ char **argv, /*const*/ char *item)
{
	for (; i < argc - 1; i++)
		argv[i] = argv[i + 1];
	argv[argc - 1] = item;
}
