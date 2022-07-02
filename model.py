import torch

class Reshape(torch.nn.Module):
    def __init__(self, shape):
        super().__init__()
        self.shape = shape

    def forward(self, x):
        return x.reshape(self.shape)

class GowGameModel(torch.nn.Module):
    def __init__(self):
        super().__init__()
        gem_embed_size = 8
        base_embed_size = 8
        out_channel = 128
        kernel_size = (5, 5)

        count_embed_size = 16
        self.base_embed = torch.nn.parameter.Parameter(torch.empty(base_embed_size))
        self.gem_embed = torch.nn.Embedding(10, gem_embed_size)
        self.counts_embed = torch.nn.Embedding(200, count_embed_size)

        self.conv2d = torch.nn.Conv2d(gem_embed_size + base_embed_size,
                                        out_channel, kernel_size)

        pooling_output = kernel_size[0] * kernel_size[1] * 8
        self.pooling_network = torch.nn.Sequential(
            Reshape((-1, out_channel)),
            torch.nn.BatchNorm1d(out_channel),
            Reshape((-1, kernel_size[0], kernel_size[1], out_channel)),
            torch.nn.Linear(out_channel, 64),
            torch.nn.ReLU(),
            torch.nn.Linear(64, 32),
            torch.nn.ReLU(),
            torch.nn.Linear(32, 8),
            Reshape((-1, pooling_output)),
        )

        self.state_network = torch.nn.Sequential(
            torch.nn.Linear(pooling_output, 128),
            torch.nn.ReLU(),
            torch.nn.Linear(128, 64),
            torch.nn.ReLU(),
            torch.nn.Linear(64, 16),
            torch.nn.ReLU(),
            torch.nn.Linear(16, 3)
        )

        self.reward_network = torch.nn.Sequential(
            torch.nn.Linear(pooling_output + count_embed_size, 128),
            torch.nn.ReLU(),
            torch.nn.Linear(128, 64),
            torch.nn.ReLU(),
            torch.nn.Linear(64, 16),
            torch.nn.ReLU(),
            torch.nn.Linear(16, 1)
        )

    def forward(self, mps, counts):
        mps = self.gem_embed(mps.cuda())
        expand_shape = (mps.shape[0], mps.shape[1], mps.shape[2], self.base_embed.shape[0])
        mps = torch.cat((mps, self.base_embed.expand(expand_shape)), dim=3)
        mps = mps.permute(0, 3, 1, 2)

        mps = self.conv2d(mps)
        mps = mps.permute(0, 2, 3, 1)
        mps = self.pooling_network(mps)
        counts = self.counts_embed(counts)
        reward_input = torch.cat((counts, mps), dim=1)
        return self.state_network(mps), self.reward_network(reward_input).reshape(mps.shape[0])
